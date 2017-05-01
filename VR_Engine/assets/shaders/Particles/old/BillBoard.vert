#version 420

uniform mat4 uModel;
uniform mat4 uView;

layout(location = 0) in vec3 in_vert;
layout(location = 1) in float in_size;
layout(location = 2) in float in_alpha;

out float size;
out float alpha;

void main()
{
	gl_Position = uView * uModel * vec4(in_vert, 1.0);
	size = in_size;
	alpha = in_alpha;

}