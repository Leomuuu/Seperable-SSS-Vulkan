#version 450

layout(binding = 0) uniform ShadowMVP {
    mat4 worldprojview;
    mat4 lightprojview;
} mvp;

layout(binding = 1) uniform dynamicUbo{
    mat4 model;
}dynamicubo;


layout(location = 0) in vec3 inPosition;


void main() {
	
	gl_Position = mvp.worldprojview* dynamicubo.model * vec4(inPosition , 1.0);

}