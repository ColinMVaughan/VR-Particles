#pragma once
#include <glew/glew.h>
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Utilities.h"
#include <vector>

class GPUParticles
{
public:
	GPUParticles();
	~GPUParticles();

	bool Init(unsigned a_ParticleNum);
	void Update(float dt, Vector3 target1, Vector3 target2, float attraction1, float attraction2);
	void Render();


public:



	void InitalizeParticles();

	//Using a crazy number of buffers because
	FrameBuffer* NewPosBuffer, *OldPosBuffer, *TempBuffer;
	FrameBuffer* NewVelBuffer, *OldVelBuffer;


	ShaderProgram PosSimulation;
	ShaderProgram VelSimulation;
	ShaderProgram Rendering;
	ShaderProgram InitalizeParticle;

	GLuint m_VAO;
	GLuint m_VBO_Index;
	unsigned int m_numParticles;
	unsigned int m_TotalParticles;
	std::vector<float> m_IndexList;
	std::vector<float> m_NewIndexList;

	bool m_PingPong;
	float acceleration = 0;
};
