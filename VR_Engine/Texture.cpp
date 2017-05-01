#include "Texture.h"
#include "SOIL\SOIL.h"

#include <iostream>
#include <vector>

Texture::~Texture()
{
	Unload();
}

bool Texture::Load(const std::string &file)
{
	TexObj = SOIL_load_OGL_texture(file.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS|SOIL_FLAG_INVERT_Y);

	if (TexObj == 0)
	{
		std::cout << "Texture failed to load.\n" << SOIL_last_result() << "\n";
		return false;
	}

	//Modify Texture
	glBindTexture(GL_TEXTURE_2D, TexObj);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //U axis
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //V axis

	return true;
}

//--------------------------------------------------------
//Loads the texture from an openVR RenderModel_Texture
//--------------------------------------------------------
bool Texture::LoadFromRenderModel(vr::RenderModel_TextureMap_t& texture)
{
	glGenTextures(1, &TexObj);
	glBindTexture(GL_TEXTURE_2D, TexObj);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.unWidth, texture.unHeight, 
		0, GL_RGBA, GL_UNSIGNED_BYTE, texture.rubTextureMapData);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat flargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &flargest);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, flargest);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool Texture::LoadCubeMap(std::vector<const GLchar*> faces )
{
	glGenTextures(1, &TexObj);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, TexObj);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
			);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return true;
}

void Texture::Unload()
{
	if (TexObj != 0)
	{
		glDeleteTextures(1, &TexObj);
		TexObj = 0;
	}
}

void Texture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, TexObj);
}

void Texture::UnBind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}