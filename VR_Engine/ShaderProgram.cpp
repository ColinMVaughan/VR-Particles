#include "ShaderProgram.h"

#include <vector>
#include <fstream>
#include <iostream>

ShaderProgram::ShaderProgram()
{

}
ShaderProgram::~ShaderProgram()
{
	if (_IsInit)
	{
		UnLoad();
	}
}


bool ShaderProgram::Load(const std::string &vertFile, const std::string &fragFile)
{
	//Create shader program objects
	_VertexShader = glCreateShader(GL_VERTEX_SHADER);
	_FragShader = glCreateShader(GL_FRAGMENT_SHADER);
	_Program = glCreateProgram();

	//Load our source code
	std::string source = ReadFile(vertFile);
	const GLchar *temp = static_cast<const GLchar *>(source.c_str());
	glShaderSource(_VertexShader, 1, &temp, NULL);

	//Load our source code
	source = ReadFile(fragFile);
	temp = static_cast<const GLchar *>(source.c_str());
	glShaderSource(_FragShader, 1, &temp, NULL);

	//compile the code
	if (!CompileShader(_VertexShader))
	{
		std::cout << "Vertex Shader failed to compile.\n";

		OutputShaderLog(_VertexShader);
		OutputProgramLog();
		UnLoad();

		return false;
	}

	if (!CompileShader(_FragShader))
	{
		std::cout << "Fragment shader failed to compile";

		OutputShaderLog(_FragShader);
		OutputProgramLog();
		UnLoad();

		return false;
	}

	glAttachShader(_Program, _VertexShader);
	glAttachShader(_Program, _FragShader);

	if (!LinkProgram())
	{
		std::cout << "Shader program failed to link. \n";

		OutputProgramLog();
		UnLoad();

		return false;
	}

	_IsInit = true;
	return true;
}

bool ShaderProgram::Load(const std::string &vertFile, const std::string &fragFile, const std::string &geomFile)
{
	//Create shader program objects
	_VertexShader = glCreateShader(GL_VERTEX_SHADER);
	_GeomShader = glCreateShader(GL_GEOMETRY_SHADER);
	_FragShader = glCreateShader(GL_FRAGMENT_SHADER);
	_Program = glCreateProgram();

	//Load our source code
	std::string source = ReadFile(vertFile);
	const GLchar *temp = static_cast<const GLchar *>(source.c_str());
	glShaderSource(_VertexShader, 1, &temp, NULL);

	//Load our source code
	source = ReadFile(fragFile);
	temp = static_cast<const GLchar *>(source.c_str());
	glShaderSource(_FragShader, 1, &temp, NULL);

	//Load our source code
	source = ReadFile(geomFile);
	temp = static_cast<const GLchar *>(source.c_str());
	glShaderSource(_GeomShader, 1, &temp, NULL);


	//compile the code
	if (!CompileShader(_VertexShader))
	{
		std::cout << "Vertex Shader failed to compile.\n";

		OutputShaderLog(_VertexShader);
		OutputProgramLog();
		UnLoad();

		return false;
	}

	if (!CompileShader(_GeomShader))
	{
		std::cout << "Geometry shader failed to compile";

		OutputShaderLog(_GeomShader);
		OutputProgramLog();
		UnLoad();

		return false;
	}

	if (!CompileShader(_FragShader))
	{
		std::cout << "Fragment shader failed to compile";

		OutputShaderLog(_FragShader);
		OutputProgramLog();
		UnLoad();

		return false;
	}

	glAttachShader(_Program, _VertexShader);
	glAttachShader(_Program, _GeomShader);
	glAttachShader(_Program, _FragShader);

	if (!LinkProgram())
	{
		std::cout << "Shader program failed to link. \n";

		OutputProgramLog();
		UnLoad();

		return false;
	}

	_IsInit = true;
	return true;
}

bool ShaderProgram::IsLoaded() const
{
	return _IsInit;
}


void ShaderProgram::UnLoad()
{
	if (_VertexShader != 0)
	{
		glDetachShader(_Program, _VertexShader);
		glDeleteShader(_VertexShader);
		_VertexShader = 0;
	}

	if (_FragShader != 0)
	{
		glDetachShader(_Program, _FragShader);
		glDeleteShader(_FragShader);
		_FragShader = 0;
	}

	if (_Program != 0)
	{
		glDeleteProgram(_Program);
		_Program = 0;
	}
	_IsInit = false;
}


bool ShaderProgram::LinkProgram()
{
	glLinkProgram(_Program);
	GLint success;
	glGetProgramiv(_Program, GL_LINK_STATUS, &success);

	return success == GL_TRUE;
}

void ShaderProgram::Bind() const
{
	glUseProgram(_Program);
}

void ShaderProgram::UnBind()
{
	glUseProgram(0);
}

void ShaderProgram::AddAttribute(unsigned int index, const std::string &attribName)
{
	glBindAttribLocation(_Program, index, attribName.c_str());
}

int ShaderProgram::GetAttribLocation(const std::string &attribName)
{
	return glGetAttribLocation(_Program, attribName.c_str());
}

int ShaderProgram::GetUniformLocation(const std::string &uniformName)
{
	return glGetUniformLocation(_Program, uniformName.c_str());
}

void ShaderProgram::SendUniform(const std::string &name, int integer)
{
	GLint location = GetUniformLocation(name);
	glUniform1i(location, integer);
}

void ShaderProgram::SendUniform(const std::string &name, unsigned int unsigned_integer)
{
	GLint location = GetUniformLocation(name);
	glUniform1ui(location, unsigned_integer);
}

void ShaderProgram::SendUniform(const std::string &name, float scalar)
{
	GLint location = GetUniformLocation(name);
	glUniform1f(location, scalar);
}

void ShaderProgram::SendUniform(const std::string& name, double scalar)
{
	GLint location = GetUniformLocation(name);
	glUniform1f(location, scalar);
}

void ShaderProgram::SendUniform(const std::string &name,  Vector2 &vector)
{
	GLint location = GetUniformLocation(name);
	glUniform2f(location, vector[0],vector[1]);
}

void ShaderProgram::SendUniform(const std::string &name,  Vector3 &vector)
{
	GLint location = GetUniformLocation(name);
	glUniform3f(location, vector[0], vector[1], vector[2]);
}

void ShaderProgram::SendUniform(const std::string &name,  Vector4 &vector)
{
	GLint location = GetUniformLocation(name);
	glUniform4f(location, vector[0], vector[1], vector[2], vector[3]);
}

void ShaderProgram::SendUniformMat3(const std::string &name, const float *matrix, bool transpose)
{
	GLint location = GetUniformLocation(name);
	glUniformMatrix3fv(location, 1, transpose,matrix);
}

void ShaderProgram::SendUniformMat4(const std::string &name, const float *matrix, bool transpose)
{
	GLint location = GetUniformLocation(name);
	glUniformMatrix4fv(location, 1, transpose, matrix);
}

std::string ShaderProgram::ReadFile(const std::string &fileName) const
{
	std::ifstream inStream(fileName);

	if (!inStream.good())
	{
		std::cout << "Shader file not found!\n";
		return std::string();
	}

	std::string data(std::istreambuf_iterator<char>(inStream), (std::istreambuf_iterator<char>()));
	return data;
}

bool ShaderProgram::CompileShader(GLuint shader) const
{
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	return success == GL_TRUE;
}

void ShaderProgram::OutputShaderLog(GLuint shader) const
{
	std::vector<char> infoLog;
	infoLog.resize(512);

	GLint infoLen;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
	glGetShaderInfoLog(shader, sizeof(char)* 512, &infoLen, &infoLog[0]);

	std::cout << std::string(infoLog.begin(), infoLog.end()) << std::endl;
}

void ShaderProgram::OutputProgramLog() const
{
	std::vector<char> infoLog;
	infoLog.resize(512);

	GLint infoLen;
	glGetProgramiv(_Program, GL_INFO_LOG_LENGTH, &infoLen);
	glGetProgramInfoLog(_Program, sizeof(char)* 512, &infoLen, &infoLog[0]);

	std::cout << std::string(infoLog.begin(), infoLog.end()) << std::endl;
}