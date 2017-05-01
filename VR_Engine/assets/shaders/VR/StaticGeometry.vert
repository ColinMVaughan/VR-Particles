#version 410

uniform mat4 MVP;

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 Normal;



out vec2 texcoord;

void main()
{
	texcoord = UV;
	gl_Position = MVP * vec4(position.xyz, 1);

}