#pragma once
#include <glew/glew.h>
#include <string>
#include <OpenVR/openvr.h>
#include <vector>

class Texture
{
public:
	Texture() = default;
	~Texture();

	bool Load(const std::string &file);
	bool LoadFromRenderModel(vr::RenderModel_TextureMap_t& texture);
	bool LoadCubeMap(std::vector<const GLchar*> faces);

	void Unload();

	void Bind();
	void UnBind();

	GLuint TexObj = 0;
};
