#include "GPUParticles.h"
#include <iostream>

GPUParticles::GPUParticles()
{

}

GPUParticles::~GPUParticles()
{	
	delete OldPosBuffer;
	delete NewPosBuffer;
}

bool GPUParticles::Init(unsigned a_ParticleNum)
{

	if (!PosSimulation.Load("./assets/Shaders/VR/PassThrough.vert", "./assets/shaders/Particles/SimulateParticlePosition.frag"))
	{
		std::cout << "\nSim Shader failed to load";
		return false;
	}

	if (!VelSimulation.Load("./assets/Shaders/VR/PassThrough.vert", "./assets/shaders/Particles/SimulateParticleVelocity.frag"))
	{
		std::cout << "\nSim Shader failed to load";
		return false;
	}

	if (!InitalizeParticle.Load("./assets/shaders/VR/PassThrough.vert", "./assets/shaders/Particles/InitalizeParticle.frag"))
	{
		std::cout << "\nSim Shader failed to load";
		return false;
	}



	OldPosBuffer = new FrameBuffer(1);
	NewPosBuffer = new FrameBuffer(1);

	OldVelBuffer = new FrameBuffer(1);
	NewVelBuffer = new FrameBuffer(1);
	

	m_numParticles = a_ParticleNum;
	m_TotalParticles = a_ParticleNum * a_ParticleNum;
	float float_num = (float)m_numParticles;

	//We add a collection of indexes for each vertex. we fill the vector with a linerly increasing index
	//m_IndexList.reserve(a_ParticleNum);
	//for (float i = 0; i < float_num; ++i)
	//{
	//	m_IndexList.push_back(i / float_num);
	//}


	//--------------------------------------------------------
	m_NewIndexList.reserve(m_TotalParticles);
	for (float y = 0; y < float_num; ++y)
	{
		for (float x = 0; x < float_num; ++x)
		{
			float u = x / float_num;
			float v = y / float_num;

			m_NewIndexList.push_back(u);
			m_NewIndexList.push_back(v);
		}
	}

	//vec2 temp(0, 0);
	//vec2 temp2(1.0f, 0);
	//vec2 temp3(0, 1.0f);
	//vec2 temp4(1.0f, 1.0f);

	//m_NewIndexList.push_back(temp);
	//m_NewIndexList.push_back(temp2);
	//m_NewIndexList.push_back(temp3);
	//m_NewIndexList.push_back(temp4);

	//------------------------------------------------------

	OldPosBuffer->InitColorTexture(0, a_ParticleNum, a_ParticleNum, GL_RGBA32F, GL_NEAREST, GL_CLAMP_TO_EDGE);
	NewPosBuffer->InitColorTexture(0, a_ParticleNum, a_ParticleNum, GL_RGBA32F, GL_NEAREST, GL_CLAMP_TO_EDGE);

	OldVelBuffer->InitColorTexture(0, a_ParticleNum, a_ParticleNum, GL_RGBA32F, GL_NEAREST, GL_CLAMP_TO_EDGE);
	NewVelBuffer->InitColorTexture(0, a_ParticleNum, a_ParticleNum, GL_RGBA32F, GL_NEAREST, GL_CLAMP_TO_EDGE);


	//---------------------------------------------------------------------------------------------------

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO_Index);
	
	glBindVertexArray(m_VAO);

	//Bind an index for each vertex in the buffer.
	//This is so we can know what position data from the texture we need to access for every giver vertex.
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Index);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* m_NewIndexList.size(), &m_NewIndexList[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (char*)0);

	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	glBindVertexArray(GL_NONE);




	InitalizeParticles();
	return true;
}


//---------------------------------------
//Clear all buffers.
//Set position buffer to random values
//---------------------------------------
void GPUParticles::InitalizeParticles()
{
	glClearColor(0, 0, 0, 0);
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, m_numParticles, m_numParticles);


	NewPosBuffer->Clear();
	OldPosBuffer->Clear();
	NewVelBuffer->Clear();
	OldVelBuffer->Clear();

	NewPosBuffer->Bind();

	InitalizeParticle.Bind();
	DrawFullScreenQuad();
	InitalizeParticle.UnBind();


	NewPosBuffer->UnBind();
}



//Here we pass the controller info to the gpu and have it simulate the particles.
void GPUParticles::Update(float dt, Vector3 target1, Vector3 target2, float attraction1, float attraction2)
{
	//Swap the new and old color buffers
	TempBuffer = NewPosBuffer;
	NewPosBuffer  = OldPosBuffer;
	OldPosBuffer  = TempBuffer;

	TempBuffer = nullptr;

	TempBuffer = NewVelBuffer;
	NewVelBuffer = OldVelBuffer;
	OldVelBuffer = TempBuffer;


	//------------------------------------------------------
	glClearColor(0, 0, 0, 0);
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, m_numParticles, m_numParticles);

	//Update Velocity
	//------------------------------------------------------
	NewVelBuffer->Clear();
	NewVelBuffer->Bind();


	//Bind textures to appropriate texture slots
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, OldVelBuffer->GetColorHandle(0));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, OldPosBuffer->GetColorHandle(0));

	VelSimulation.Bind();
	VelSimulation.SendUniform("TargetPos", target1);
	VelSimulation.SendUniform("AttractionStrength", attraction1);

	VelSimulation.SendUniform("TargetPos2", target2);
	VelSimulation.SendUniform("AttractionStrength2", attraction2);

	VelSimulation.SendUniform("prevVelocity", 0);
	VelSimulation.SendUniform("prevPosition", 1);
	VelSimulation.SendUniform("deltaTime", dt);

	DrawFullScreenQuad();

	VelSimulation.UnBind();

	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	NewVelBuffer->UnBind();

	//Update Positions
	//--------------------------------------------------------

	//Clear and bind position buffer
	NewPosBuffer->Clear();
	NewPosBuffer->Bind();

	//Bind textures to appropriate texture slots
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, OldPosBuffer->GetColorHandle(0));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NewVelBuffer->GetColorHandle(0));

	PosSimulation.Bind();

	PosSimulation.SendUniform("prevPosition", 0);
	PosSimulation.SendUniform("prevVelocity", 1);
	PosSimulation.SendUniform("deltaTime", dt);

	DrawFullScreenQuad();	//Draw
	PosSimulation.UnBind();

	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	NewPosBuffer->UnBind();

	glEnable(GL_DEPTH_TEST);
}

//Render the Particles after they have been simulated
void GPUParticles::Render()
{



}
