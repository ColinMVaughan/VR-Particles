#version 410

in vec2 texcoord;
out vec4 outColor;


uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

float Random(float x)
{
    const uint mantissaMask = 0x007FFFFFu;
    const uint one          = 0x3F800000u;
    
    uint h = hash( floatBitsToUint( x ) );
    h &= mantissaMask;
    h |= one;
    
    float  r2 = uintBitsToFloat( h );
    return r2 - 1.0;
}


void main()
{
	float x = Random(texcoord.x);
	float y = Random(texcoord.y);
	float z = Random(texcoord.x + texcoord.y);
	vec3 pos = vec3(x,y,z);
	
	pos = pos * 1;
	outColor = vec4(pos, 1);

}



