#version 410

uniform sampler2D prevVelocity;
uniform sampler2D prevPosition;
uniform float deltaTime;

uniform vec3 TargetPos;
uniform float AttractionStrength;

uniform vec3 TargetPos2;
uniform float AttractionStrength2;

in vec2 texcoord;
out vec4 outColor;


void main()
{
	vec3 Velocity;
	vec3 Acceleration;
	float Friction = 0.99;

	
	vec3 prevVel = texture(prevVelocity, texcoord).rgb;
	vec3 prevPos = texture(prevPosition, texcoord).rgb;
	
	vec3 dir = TargetPos - prevPos;
	vec3 dir2 = TargetPos2 - prevPos;
	
	
	Acceleration = ((dir * (AttractionStrength / (length(dir) * length(dir)))) + (dir2 * (AttractionStrength2 / (length(dir2) * length(dir2))))) * deltaTime;
	Acceleration *= 0.03;
	
	if(length(dir) < 0.005 || length(dir2) < 0.005)
	{
		Acceleration *= 0;
	}
	
	Velocity = (prevVel + Acceleration);
	Velocity *= Friction;
	
	
	//Velocity = clamp(Velocity, vec3(-0.01,-0.01,-0.01), vec3(0.01,0.01,0.01));
	
	outColor = vec4(Velocity, 1);
	

}