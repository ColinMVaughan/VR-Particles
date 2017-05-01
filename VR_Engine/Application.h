#pragma once
#include<sdl2-2.0.3/SDL.h>

#include "ShaderProgram.h"
#include "Mesh.h"
#include "Texture.h"
#include "VrApp.h"
#include "Timer.h"
#include "ParticleEffect.h"
#include "GPUParticles.h"

class Application
{
public:
	Application();
	~Application();


	bool Initalize();
	void RunMainLoop();
	bool Shutdown();

private:

	void LoadSkyBox();
	Texture bla;

	void Update();
	void Render();
	void Draw(vr::Hmd_Eye eye);
	void DrawForWindow();

private:
	float TotalGameTime = 0.0f;
	bool Running = true;

	VrApp HMD;

	Mesh* Cube;
	Texture CubeTex;

	GPUParticles Particles;

	ShaderProgram DefaultShader;
	ShaderProgram ParticleShader;

	Uint64 CurrentTime;
	Uint64 LastTime;
	float deltatime;

	FrameBuffer* windowBuffer;
	Vector3 camera;

};
