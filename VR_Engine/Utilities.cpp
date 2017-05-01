#include "Utilities.h"
#include <glew/glew.h>

#define BUFFER_OFFSET(i) ((char *) + (i))

GLuint _FullScreenQuadVAO = GL_NONE;
GLuint _FullScreenQuadVBO = GL_NONE;

void InitFullScreenQuad()
{
	float VBO_DATA[] = 
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,

		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
	};

	int vertexSize = 6 * 3 * sizeof(float);
	int texCoordSize = 6 * 2 * sizeof(float);

	glGenVertexArrays(1, &_FullScreenQuadVAO);
	glBindVertexArray(_FullScreenQuadVAO);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &_FullScreenQuadVBO);

	glBindBuffer(GL_ARRAY_BUFFER, _FullScreenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, vertexSize + texCoordSize, VBO_DATA, GL_STATIC_DRAW);

	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertexSize));

	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	glBindVertexArray(GL_NONE);
}

void DrawFullScreenQuad()
{
	glBindVertexArray(_FullScreenQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(GL_NONE);
}


float LERP(float a, float b, float interp)
{
	return 0;
}