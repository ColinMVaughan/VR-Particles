#include "ParticleEffect.h"
#include "Random.h"
#include "Utilities.h"

#define BUFFER_OFFSET(i) ((char *)0 + (i))

ParticleEffect::ParticleEffect()
{
}

ParticleEffect::~ParticleEffect()
{
	if (_VAO != GL_NONE)
	{
		glDeleteVertexArrays(1, &_VAO);
	}

	if (_VBO_Position != GL_NONE)
	{
		glDeleteBuffers(1, &_VBO_Position);
	}

	if (_VBO_Size != GL_NONE)
	{
		glDeleteBuffers(1, &_VBO_Size);
	}

	if (_VBO_Alpha != GL_NONE)
	{
		glDeleteBuffers(1, &_VBO_Alpha);
	}

	if (_Particles.Positions != nullptr)
	{
		delete[] _Particles.Positions;
		delete[] _Particles.Velocities;
		delete[] _Particles.Alpha;
		delete[] _Particles.Ages;
		delete[] _Particles.Lifetimes;
		delete[] _Particles.Size;
	}
}

bool ParticleEffect::Init(const std::string &textureFile, unsigned int maxParticles, unsigned int rate)
{
	if (!_Texture.Load(textureFile))
	{
		return false;
	}

	_MaxParticles = maxParticles;
	_Rate = rate;

	_Particles.Positions = new Vector3[_MaxParticles];
	_Particles.Velocities = new Vector3[_MaxParticles];
	_Particles.Alpha = new float[_MaxParticles];
	_Particles.Ages = new float[_MaxParticles];
	_Particles.Lifetimes = new float[_MaxParticles];
	_Particles.Size = new float[_MaxParticles];

	//Setup Opengl Memory
	glGenVertexArrays(1, &_VAO);
	glGenBuffers(1, &_VBO_Position);
	glGenBuffers(1, &_VBO_Size);
	glGenBuffers(1, &_VBO_Alpha);

	glBindVertexArray(_VAO);

	glEnableVertexAttribArray(0);	//Vertex
	glEnableVertexAttribArray(1);	//Size
	glEnableVertexAttribArray(2);	//Alpha

	glBindBuffer(GL_ARRAY_BUFFER, _VBO_Position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3)* _MaxParticles, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, _VBO_Size);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _MaxParticles, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer((GLuint)1, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, _VBO_Alpha);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _MaxParticles, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer((GLuint)2, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	glBindVertexArray(GL_NONE);


	return true;
}

void ParticleEffect::Update(float elapsed)
{
	int NumToSpawn = _Rate;

	/// Create new particles ///
	while (
		//We have not reached the particle cap and...
		_NumCurrentParticles < _MaxParticles &&
		//We have more particles to generate this frame...
		NumToSpawn > 0)
	{
		_Particles.Alpha[_NumCurrentParticles]		= LerpAlpha[0];
		_Particles.Ages[_NumCurrentParticles]		= 0.0f;
		_Particles.Lifetimes[_NumCurrentParticles]	= RandomRangef(RangeLifetime[0], RangeLifetime[1]);
		_Particles.Size[_NumCurrentParticles]		= LerpSize[0];
		_Particles.Positions[_NumCurrentParticles].set( RandomRangef(RangeX[0], RangeX[1]), RandomRangef(RangeY[0], RangeY[1]), RandomRangef(RangeZ[0], RangeZ[1]));

		//send the particle in a random direction, with a velocity between our range
		_Particles.Velocities[_NumCurrentParticles].set( RandomRangef(DirectionRangeX[0], DirectionRangeX[1]), RandomRangef(DirectionRangeY[0], DirectionRangeY[1]), RandomRangef(DirectionRangeZ[0], DirectionRangeZ[1]));
		_Particles.Velocities[_NumCurrentParticles].normalize();
		_Particles.Velocities[_NumCurrentParticles] *= RandomRangef(RangeVelocity[0], RangeVelocity[1]);

		//counters...
		_NumCurrentParticles++;
		NumToSpawn--;
	}

	/// Update existing particles ///
	for (unsigned i = 0; i < _NumCurrentParticles; i++)
	{
		_Particles.Ages[i] += elapsed;

		if (_Particles.Ages[i] > _Particles.Lifetimes[i])
		{
			//remove the particle by replacing it with the one at the top of the stack
			_Particles.Alpha[i]		= _Particles.Alpha[_NumCurrentParticles - 1];
			_Particles.Ages[i]		= _Particles.Ages[_NumCurrentParticles - 1];
			_Particles.Lifetimes[i] = _Particles.Lifetimes[_NumCurrentParticles - 1];
			_Particles.Positions[i] = _Particles.Positions[_NumCurrentParticles - 1];
			_Particles.Size[i]		= _Particles.Size[_NumCurrentParticles - 1];
			_Particles.Velocities[i] = _Particles.Velocities[_NumCurrentParticles - 1];

			_NumCurrentParticles--;
			continue;
		}


		_Particles.Velocities[i] += ConstantVelocity;
		_Particles.Positions[i] += _Particles.Velocities[i] * elapsed;

		float interp = _Particles.Ages[i] / _Particles.Lifetimes[i];
		
		_Particles.Alpha[i] = LERP(LerpAlpha[0], LerpAlpha[1], interp);
		_Particles.Size[i]	= LERP(LerpSize[0], LerpSize[1], interp);
	}

	//update OpenGL on the changes
	glBindBuffer(GL_ARRAY_BUFFER, _VBO_Position);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vector3)* _NumCurrentParticles, &_Particles.Positions[0]);

	glBindBuffer(GL_ARRAY_BUFFER, _VBO_Size);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)* _NumCurrentParticles, &_Particles.Size[0]);

	glBindBuffer(GL_ARRAY_BUFFER, _VBO_Alpha);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)* _NumCurrentParticles, &_Particles.Alpha[0]);

	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
}

void ParticleEffect::Render()
{
	if (_NumCurrentParticles == 0)
	{
		return;
	}

	glPointSize((GLfloat)PointSize);

	_Texture.Bind();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glBindVertexArray(_VAO);
	glDrawArrays(GL_POINTS, 0, _NumCurrentParticles);
	glBindVertexArray(GL_NONE);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	_Texture.UnBind();
}
