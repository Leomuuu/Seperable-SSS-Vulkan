#version 450

layout(binding = 0) uniform ShadowMVP {
    mat4 worldprojview;
    mat4 lightprojview;
} mvp;

layout(binding = 1) uniform dynamicUbo{
    mat4 model;
}dynamicubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec4 shadowCoord;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out vec4 shadowCoordShrink;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );


void main() {
	
	gl_Position = mvp.worldprojview * dynamicubo.model * vec4(inPosition , 1.0);
	
	fragPos = (dynamicubo.model *vec4(inPosition,1.0)).xyz;
    fragNormal = mat3(dynamicubo.model) *inNormal;
	fragTexCoord = inTexCoord;

    shadowCoord = biasMat*mvp.lightprojview * dynamicubo.model * vec4(inPosition,1.0);

	vec4 shrinkedPos = vec4(fragPos - 0.005 * fragNormal, 1.0);
	shadowCoordShrink=biasMat*mvp.lightprojview *shrinkedPos;
}