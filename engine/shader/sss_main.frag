#version 450

layout(location = 0) in vec4 shadowCoord;
layout(location = 1) in vec4 lightCoord;

layout(binding = 2) uniform sampler2D lightSampler;
layout(binding = 3) uniform sampler2D shadowMapSampler;

layout(location = 0) out vec4 outColor;

// shadow
float GetShadow(vec4 shadowCoordN , vec2 offset){

    float shadow=1.0;
    if ( shadowCoordN.z > -1.0 && shadowCoordN.z < 1.0 ) 
	{
		float dist = texture( shadowMapSampler, shadowCoordN.xy+offset).r;
		if ( shadowCoordN.w > 0.0 && dist < shadowCoordN.z ) 
		{
			shadow = 0.0;
		}
	}
    return shadow;
}
float PCF(vec4 shadowCoordN){

    ivec2 texDim = textureSize(shadowMapSampler, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 2;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += GetShadow(shadowCoordN, vec2(dx*x, dy*y));
			count++;
		}
	}
	return shadowFactor / count;
}


void main() {
	// shadow
    vec4 shadowCoordN=shadowCoord/shadowCoord.w;
    float shadow=PCF( shadowCoordN );

	// light
	vec4 light=texture(lightSampler,lightCoord.xy/2+vec2(0.5));

	outColor=shadow*light;

}