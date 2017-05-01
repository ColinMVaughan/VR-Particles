#version 410


uniform sampler2D prevPosition;
uniform sampler2D prevVelocity;

uniform float deltaTime;


in vec2 texcoord;
out vec4 outColor;


void main()
{

	vec3 prevPos = texture(prevPosition, texcoord).rgb;
	vec3 prevVel = texture(prevVelocity, texcoord).rgb;
	
	vec3 Position = prevPos + prevVel;
	
	outColor = vec4(Position, 1);
	

}