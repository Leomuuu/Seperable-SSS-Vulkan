#version 450

layout(location = 0) in vec4 shadowCoord;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in vec4 shadowCoordShrink;

layout(binding = 2) uniform sampler2D lightSampler[4];
layout(binding = 3) uniform sampler2D shadowMapSampler;

layout(push_constant) uniform PushConsts {
	vec2 windowSize;
	vec3 translucency_sssWidth_lightFarPlane;
	vec3 lightWorldPosition;
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

// transmittance
vec4 GetTransmittance(float Translucency) {

     float scale = 825 * (1.0-Translucency) 
    / pushConsts.translucency_sssWidth_lightFarPlane.y;
       
	vec4 shadowCoordN=shadowCoordShrink/shadowCoordShrink.w;
	float d1=texture(shadowMapSampler, shadowCoordN.xy).r;
	float d2=shadowCoordN.z ;
    float d =  scale * abs(d1-d2);

    float dd = -d * d;
    vec3 profile =	 vec3(0.233, 0.455, 0.649) * exp(dd / 0.0064) +
                     vec3(0.1,   0.336, 0.344) * exp(dd / 0.0484) +
                     vec3(0.118, 0.198, 0.0)   * exp(dd / 0.187)  +
                     vec3(0.113, 0.007, 0.007) * exp(dd / 0.567)  +
                     vec3(0.358, 0.004, 0.0)   * exp(dd / 1.99)   +
                     vec3(0.078, 0.0,   0.0)   * exp(dd / 7.41)
					;

    vec3 light= pushConsts.lightWorldPosition -fragPos;

    return vec4(profile*clamp(0.3+dot(light, -fragNormal),0.0,1.0),1.0);
}


void main() {
	// shadow
    vec4 shadowCoordN=shadowCoord/shadowCoord.w;
    float shadow=PCF( shadowCoordN );

	// light
	vec2 lightCoord=vec2(gl_FragCoord.x/pushConsts.windowSize.x,gl_FragCoord.y/pushConsts.windowSize.y);
	vec4 light0=vec4(texture(lightSampler[0],lightCoord.xy).xyz,1.0);
	vec4 light1=vec4(texture(lightSampler[1],lightCoord.xy).xyz,1.0);
	vec4 lightTranslucency=vec4(texture(lightSampler[2],fragTexCoord).xyz,1.0);
	vec4 light3=vec4(texture(lightSampler[3],fragTexCoord).xyz,1.0);

	vec3 light= pushConsts.lightWorldPosition -fragPos;
	outColor=(light0+light1) *shadow + 
		lightTranslucency * GetTransmittance(lightTranslucency.r+pushConsts.translucency_sssWidth_lightFarPlane.x)
			* light3 * shadow;

}