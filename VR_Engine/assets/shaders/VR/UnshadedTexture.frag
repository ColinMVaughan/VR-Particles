#version 410

uniform sampler2D myTexture;
in vec2 texcoord;

out vec4 outColor;

void main()
{
	outColor = texture(myTexture, texcoord);
}