#include "Application.h"

Application::Application() :
	HMD()
{
}

Application::~Application()
{
}


bool Application::Initalize()
{
	HMD.InitVr();

	Cube = new Mesh();
	Cube->LoadFromFile("./assets/models/grid.obj");
	CubeTex.Load("./assets/textures/cube.png");

	if (!DefaultShader.Load("./assets/shaders/VR/StaticGeometry.vert", "./assets/shaders/VR/UnshadedTexture.frag"))
	{
		std::cout << "\nFailed to load default shader.";
	}
	if (!ParticleShader.Load("./assets/shaders/Particles/RenderParticle.vert", "./assets/shaders/Particles/SolidColor.frag"))
	{
		std::cout << "\nFailed to load default shader.";
	}
	windowBuffer = new FrameBuffer(1);
	windowBuffer->InitColorTexture(0, HMD.m_unCompanionWindowWidth, HMD.m_unCompanionWindowHeight, GL_RGBA8, GL_NEAREST, GL_CLAMP_TO_EDGE);
	windowBuffer->InitDepthTexture(HMD.m_unCompanionWindowWidth, HMD.m_unCompanionWindowHeight);

	Particles.Init(500);

	return true;
}

void Application::RunMainLoop()
{
	while (Running)
	{
		Update();
		Render();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			/* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					Running = false;
				break;

			default:
				break;
			}
		}

	}
}



void Application::Update()
{
	LastTime = CurrentTime;
	CurrentTime = SDL_GetTicks();
	deltatime = (float)(CurrentTime - LastTime) / 1000.0;
	TotalGameTime += deltatime;

	HMD.HandleInput();


	Vector4 target1(0, 0, 0, 1);
	Vector4 target2(0, 0, 0, 1);

	vr::VRControllerState_t controllerState1;
	vr::VRControllerState_t controllerState2;

	target1 = HMD.GetTrackedDevicePose(0) * target1;
	target2 = HMD.GetTrackedDevicePose(1) * target2;

	HMD.GetControllerState(0, &controllerState1);
	HMD.GetControllerState(1, &controllerState2);

	float acceleration1 = (controllerState1.rAxis[1].x) * 0.2f;
	float acceleration2 = (controllerState2.rAxis[1].x) * 0.2f;
	

	Particles.Update(deltatime, Vector3(target1.x, target1.y, target1.z), Vector3(target2.x, target2.y, target2.z), acceleration1, acceleration2);
}



void Application::Render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	HMD.UpdateHMDMatrixPose();
	//---------------------------------------------------------------------------
	//leftEye
	//---------------------------------------------------------------------------
	glEnable(GL_MULTISAMPLE);

	HMD.leftEyeBuffer.BindRender();
	glViewport(0, 0, HMD.m_nRenderWidth, HMD.m_nRenderHeight);

	Draw(vr::Eye_Left);

	HMD.leftEyeBuffer.UnBind();


	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, HMD.leftEyeBuffer.m_nRenderFramebufferId); //bind render eye
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, HMD.leftEyeBuffer.m_nResolveFramebufferId); //bind resolve eye

	glBlitFramebuffer(0, 0, HMD.m_nRenderWidth, HMD.m_nRenderHeight, 0, 0, HMD.m_nRenderWidth, HMD.m_nRenderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); //unbind render eye
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); //unbind resolve eye


	//----------------------------------------------------------------------------
	//rightEye
	//---------------------------------------------------------------------------
	glEnable(GL_MULTISAMPLE);

	HMD.rightEyeBuffer.BindRender();
	glViewport(0, 0, HMD.m_nRenderWidth, HMD.m_nRenderHeight);

	Draw(vr::Eye_Right);

	HMD.rightEyeBuffer.UnBind();


	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, HMD.rightEyeBuffer.m_nRenderFramebufferId); //bind render eye
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, HMD.rightEyeBuffer.m_nResolveFramebufferId); //bind resolve eye

	glBlitFramebuffer(0, 0, HMD.m_nRenderWidth, HMD.m_nRenderHeight, 0, 0, HMD.m_nRenderWidth, HMD.m_nRenderHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); //unbind render eye
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); //unbind resolve eye


	//----------------------------------------------------------------------------
	glViewport(0, 0, HMD.m_unCompanionWindowWidth, HMD.m_unCompanionWindowHeight);
	//glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	//windowBuffer->Clear();

	windowBuffer->Bind();
	DrawForWindow();
	windowBuffer->UnBind();

	HMD.RenderCompanionWindow(windowBuffer->GetColorHandle(0), HMD.m_unCompanionWindowWidth, HMD.m_unCompanionWindowHeight);
	HMD.SubmitRender(); //Submit the Render Textures to the HMD
}

void Application::Draw(vr::Hmd_Eye eye)
{
	Matrix4 HMD_MVP = HMD.GetCurrentViewProjectionMatrix(eye);
	Matrix4 transform;
	transform = HMD_MVP * transform;


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	DefaultShader.Bind();
	DefaultShader.SendUniformMat4("MVP", HMD_MVP.get(), false);
	DefaultShader.SendUniform("myTexture", 0);

	CubeTex.Bind();
	glBindVertexArray(Cube->VAO);
	glDrawArrays(GL_TRIANGLES, 0, Cube->GetNumVertices());
	glBindVertexArray(0);
	CubeTex.UnBind();

	DefaultShader.UnBind();

	//-----------------------------------------------------------------------------------
	ParticleShader.Bind();
	ParticleShader.SendUniform("StateInfo", 0);
	ParticleShader.SendUniformMat4("MVP", HMD_MVP.get(),false);
	ParticleShader.SendUniform("color", Vector3(0.5, 1, 0.5));

	glBindTexture(GL_TEXTURE_2D, Particles.NewPosBuffer->GetColorHandle(0));
	glBindVertexArray(Particles.m_VAO);

	glDrawArrays(GL_POINTS, 0, Particles.m_TotalParticles);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	ParticleShader.UnBind();

	HMD.RenderTrackedDevices(eye);
	
}

void Application::DrawForWindow()
{
	Matrix4 HMD_MVP = HMD.GetHMDMatrixProjectionEye(vr::Eye_Left);
	Matrix4 view;
	//view = HMD.GetTrackedDevicePose(0);

	view.translate(0, 1.5, 1.5f);
	view.rotateY(TotalGameTime * 3.0f);

	HMD_MVP = HMD_MVP * view.invert();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	DefaultShader.Bind();
	DefaultShader.SendUniformMat4("MVP", HMD_MVP.get(), false);
	DefaultShader.SendUniform("myTexture", 0);

	CubeTex.Bind();
	glBindVertexArray(Cube->VAO);
	glDrawArrays(GL_TRIANGLES, 0, Cube->GetNumVertices());
	glBindVertexArray(0);
	CubeTex.UnBind();

	DefaultShader.UnBind();


	//-----------------------------------------------------------------------------------
	ParticleShader.Bind();
	ParticleShader.SendUniform("StateInfo", 0);
	ParticleShader.SendUniformMat4("MVP", HMD_MVP.get(), false);
	ParticleShader.SendUniform("color", Vector3(0.5, 1, 0.5));

	glBindTexture(GL_TEXTURE_2D, Particles.NewPosBuffer->GetColorHandle(0));
	glBindVertexArray(Particles.m_VAO);

	glDrawArrays(GL_POINTS, 0, Particles.m_TotalParticles);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	ParticleShader.UnBind();

	HMD.RenderTrackedDevices(HMD_MVP, true);
}

bool Application::Shutdown()
{
	HMD.Shutdown();
	return true;
}

