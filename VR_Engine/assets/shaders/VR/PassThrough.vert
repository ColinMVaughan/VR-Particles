#version 410

uniform mat4 matrix;

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 UV;

out vec2 texcoord;

void main()
{
	texcoord = UV;
	gl_Position = position;

}