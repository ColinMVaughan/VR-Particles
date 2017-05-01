#pragma once

#include <vector>
#include "Matrices.h"
#include "Vectors.h"
#include "Texture.h"

struct ParticleList
{
	Vector3 *Positions = nullptr;
	Vector3 *Velocities = nullptr;
	float *Size = nullptr;
	float *Alpha = nullptr;
	float *Ages = nullptr;
	float *Lifetimes = nullptr;
};

class ParticleEffect
{
public:
	ParticleEffect();
	~ParticleEffect();

	bool Init(const std::string &textureFile, unsigned int maxParticles, unsigned int rate);

	void Update(float elapsed);
	void Render();
	//void SetDirection(vec3 dir);
	//void AddVelocity(vec3 dir, float scale);

	Matrix4 Transform;

	Vector2 RangeX = Vector2( -10.0f, 10.0f );
	Vector2 RangeY = Vector2( 0.0f, 10.0f );
	Vector2 RangeZ = Vector2( -10.0f, 10.0f );
	Vector2 RangeVelocity = Vector2( 0.1f, 1.0f );
	Vector2 RangeLifetime = Vector2( 7.0f, 9.5f );

	Vector2 DirectionRangeX = Vector2(-1.0, 1.0);
	Vector2 DirectionRangeY = Vector2(-1.0, 1.0);
	Vector2 DirectionRangeZ = Vector2(-1.0, 1.0);
	Vector3 ConstantVelocity = Vector3( 0.0, 0.0, 0.0 );
	//Lerp variables are dynamic based on lifetime of particle
	Vector2 LerpAlpha = Vector2( 0.5f, 0.0f );
	Vector2 LerpSize = Vector2( 0.0f, 1.0f );
	float PointSize = 5;

private:
	ParticleList _Particles;
	Texture _Texture;

	float _Rate = 0.0f;
	unsigned int _MaxParticles = 0;
	unsigned int _NumCurrentParticles = 0;

	GLuint _VAO;
	GLuint _VBO_Position;
	GLuint _VBO_Size;
	GLuint _VBO_Alpha;
};
