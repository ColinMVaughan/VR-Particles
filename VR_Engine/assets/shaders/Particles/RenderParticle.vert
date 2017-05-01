#version 410

layout(location = 0) in vec2 index;

uniform mat4 MVP;


uniform sampler2D StateInfo;

out vec2 uv;

void main()
{
	uv = index;
	vec3 pos = texture(StateInfo, index).rgb;
	gl_Position = MVP * vec4(pos,1);
}