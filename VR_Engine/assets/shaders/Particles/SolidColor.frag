#version 410

in vec2 uv;
uniform sampler2D info;

out vec4 outColor;

void main()
{

	outColor = texture(info, uv);
}