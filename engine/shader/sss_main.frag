#version 450

layout(location = 0) in vec4 shadowCoord;

layout(binding = 2) uniform sampler2D lightSampler[2];
layout(binding = 3) uniform sampler2D shadowMapSampler;

layout(push_constant) uniform PushConsts {
	vec2 windowSize;
} pushConsts;

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
	vec2 lightCoord=vec2(gl_FragCoord.x/pushConsts.windowSize.x,gl_FragCoord.y/pushConsts.windowSize.y);
	vec4 light0=vec4(texture(lightSampler[0],lightCoord.xy).xyz,1.0);
	vec4 light1=vec4(texture(lightSampler[1],lightCoord.xy).xyz,1.0);

	outColor= (light0+light1)*shadow;

}