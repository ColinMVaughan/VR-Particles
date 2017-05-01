#pragma once 
#include <vector>
#include <glew/glew.h>
#include <OpenVR/openvr.h>

class Mesh
{
public:
	Mesh();
	~Mesh();

	bool LoadFromFile(const std::string &file);
	bool InstanceMesh(unsigned int size, unsigned int elements, float data[]);
	bool LoadFromRenderModel(vr::RenderModel_t& vrModel);

	void Unload();

	unsigned int GetNumFaces() const;
	unsigned int GetNumVertices() const;
	std::string GetName();

	GLuint VBO_Verticies = 0;
	GLuint VBO_UVs		 = 0;
	GLuint VBO_Normals	 = 0;
	GLuint VAO			 = 0;
	GLuint IndexBuffer   = 0;
	GLuint VBO_InstanceBuffer = 0;

private:
	unsigned int _NumFaces = 0;
	unsigned int _NumVertices = 0;
	std::string name;

};
