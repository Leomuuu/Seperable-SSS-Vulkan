#version 450

layout(binding = 0) uniform ShadowMVP {
    mat4 worldprojview;
    mat4 lightprojview;
} mvp;

layout(binding = 1) uniform dynamicUbo{
    mat4 model;
}dynamicubo;


layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 shadowCoord;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );


void main() {
	
	gl_Position = mvp.worldprojview * dynamicubo.model * vec4(inPosition , 1.0);
	
    shadowCoord = biasMat*mvp.lightprojview * dynamicubo.model * vec4(inPosition,1.0);
}