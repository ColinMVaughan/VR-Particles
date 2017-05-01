#define _CRT_SECURE_NO_WARNINGS
#include "Mesh.h"
#include <fstream>
#include <iostream>

#include "shared/Vectors.h"

#define CHAR_BUFFER_SIZE 128
#define BUFFER_OFFSET(i) ((char *)0 + (i))

struct MeshFace
{
	MeshFace()
	{
		verticies[0] = 0;
		verticies[1] = 0;
		verticies[2] = 0;

		textureUVs[0] = 0;
		textureUVs[1] = 0;
		textureUVs[2] = 0;

		normals[0] = 0;
		normals[1] = 0;
		normals[2] = 0;
	}

	MeshFace(unsigned v1, unsigned v2, unsigned v3,
		unsigned t1, unsigned t2, unsigned t3,
		unsigned n1, unsigned n2, unsigned n3)
	{
		verticies[0] = v1;
		verticies[1] = v2;
		verticies[2] = v3;

		textureUVs[0] = t1;
		textureUVs[1] = t2;
		textureUVs[2] = t3;

		normals[0] = n1;
		normals[1] = n2;
		normals[2] = n3;
	}

	unsigned verticies[3];
	unsigned textureUVs[3];
	unsigned normals[3];

};

Mesh::Mesh()
{

}

Mesh::~Mesh()
{

}

//--------------------------------------------------------------------------
//Loads a mesh from a .obj file and attach it to a VBO / VAO.
//--------------------------------------------------------------------------
//TODO: Could be optimized as one ontiguous stream with an index buffer.
//---------------------------------------------------------------------------

bool Mesh::LoadFromFile(const std::string &file)
{
	std::ifstream input;
	input.open(file);

	if (!input)
	{
		std::cout << "Could Not Open File." << std::endl;
		return false;
	}

	char inputString[CHAR_BUFFER_SIZE];

	//Unique data
	std::vector<Vector3> vertexData;
	std::vector<Vector2> textureData;
	std::vector<Vector3> normalData;
	//Index / Face data
	std::vector<MeshFace> faceData;
	//OpenGL Ready
	std::vector<float> unPackedVertexData;
	std::vector<float> unPackedTextureData;
	std::vector<float> unPackedNormalData;

	while (!input.eof())
	{
		input.getline(inputString, CHAR_BUFFER_SIZE);

		if (std::strstr(inputString, "#") != nullptr)
		{
			//this line is a comment 
			continue;
		}
		else if (std::strstr(inputString, "vt") != nullptr)
		{
			Vector2 temp;
			std::sscanf(inputString, "vt %f %f", &temp.x, &temp.y);
			textureData.push_back(temp);
		}
		else if (std::strstr(inputString, "vn") != nullptr)
		{
			Vector3 temp;
			std::sscanf(inputString, "vn %f %f %f", &temp.x, &temp.y, &temp.z);
			normalData.push_back(temp);
		}
		else if (std::strstr(inputString, "v") != nullptr)
		{
			Vector3 temp;
			std::sscanf(inputString, "v %f %f %f", &temp.x, &temp.y, &temp.z);
			vertexData.push_back(temp);
		}
		else if (std::strstr(inputString, "f") != nullptr)
		{
			MeshFace temp;

			std::sscanf(inputString, "f %u/%u/%u %u/%u/%u %u/%u/%u",
				&temp.verticies[0], &temp.textureUVs[0], &temp.normals[0],
				&temp.verticies[1], &temp.textureUVs[1], &temp.normals[1],
				&temp.verticies[2], &temp.textureUVs[2], &temp.normals[2]);

			faceData.push_back(temp);
		}

	}

	for (unsigned i = 0; i < faceData.size(); i++)
	{
		for (unsigned j = 0; j < 3; j++)
		{
			unPackedVertexData.push_back(vertexData[faceData[i].verticies[j] - 1].x);
			unPackedVertexData.push_back(vertexData[faceData[i].verticies[j] - 1].y);
			unPackedVertexData.push_back(vertexData[faceData[i].verticies[j] - 1].z);

			unPackedTextureData.push_back(textureData[faceData[i].textureUVs[j] - 1].x);
			unPackedTextureData.push_back(textureData[faceData[i].textureUVs[j] - 1].y);

			unPackedNormalData.push_back(normalData[faceData[i].normals[j] - 1].x);
			unPackedNormalData.push_back(normalData[faceData[i].normals[j] - 1].y);
			unPackedNormalData.push_back(normalData[faceData[i].normals[j] - 1].z);
		}
	}


	_NumFaces = faceData.size();
	_NumVertices = _NumFaces * 3;

	//Send Data to GPU//

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO_Verticies);
	glGenBuffers(1, &VBO_UVs);
	glGenBuffers(1, &VBO_Normals);

	glBindVertexArray(VAO);

	glEnableVertexAttribArray(0); //Vertex
	glEnableVertexAttribArray(1); //Normals
	glEnableVertexAttribArray(2); //UVs


	glBindBuffer(GL_ARRAY_BUFFER, VBO_Verticies);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* unPackedVertexData.size(), &unPackedVertexData[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(float)* 3, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, VBO_UVs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* unPackedTextureData.size(), &unPackedTextureData[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, sizeof(float)* 2, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* unPackedNormalData.size(), &unPackedNormalData[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, sizeof(float)* 3, BUFFER_OFFSET(0));

	//Cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	vertexData.clear();
	textureData.clear();
	normalData.clear();
	faceData.clear();
	unPackedVertexData.clear();
	unPackedTextureData.clear();
	unPackedNormalData.clear();

	return true;
}

bool Mesh::InstanceMesh(unsigned int size, unsigned int elements, float data[])
{
	glGenBuffers(1, &VBO_InstanceBuffer);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_InstanceBuffer);

	glEnableVertexAttribArray(3); //Instance
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* size, &data[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)3, elements, GL_FLOAT, GL_FALSE, sizeof(float) * elements, BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(3, 1);


	glBindVertexArray(0);

	return true;
}

//-------------------------------------------
//Loads a mesh from an openVR RenderModel
//--------------------------------------------
bool Mesh::LoadFromRenderModel(vr::RenderModel_t& vrModel)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1,&VBO_Verticies);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Verticies);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); //Vertex
	glEnableVertexAttribArray(1); //UV
	glEnableVertexAttribArray(2); //Normal


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, vPosition));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, vNormal));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));


	glGenBuffers(1, &IndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	_NumFaces = vrModel.unTriangleCount;
	_NumVertices = _NumFaces * 3;

	return true;
}

void Mesh::Unload()
{
	glDeleteBuffers(1, &VBO_Normals);
	glDeleteBuffers(1, &VBO_UVs);
	glDeleteBuffers(1, &VBO_Verticies);
	glDeleteVertexArrays(1, &VAO);

	VBO_Normals		= 0;
	VBO_UVs			= 0;
	VBO_Verticies	= 0;
	VAO				= 0;

	_NumFaces		= 0;
	_NumVertices	= 0;
}

unsigned int Mesh::GetNumFaces() const
{
	return _NumFaces;
}

unsigned int Mesh::GetNumVertices() const
{
	return _NumVertices;
}

std::string Mesh::GetName()
{
	return name;
}
