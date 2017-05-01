#version 420

in vec2 uv;
out vec4 outColor;
uniform sampler2D StateInfo;

void main()
{
	vec3 color = texture(StateInfo, vec2(0,0)).rgb;
	outColor = vec4(color, 1.0);
}