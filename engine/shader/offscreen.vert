#version 450

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform MVPMatrix {
    mat4 view;
    mat4 proj;
} mvp;

layout(binding = 1) uniform dynamicUbo{
    mat4 model;
}dynamicubo;
 
void main()
{
	gl_Position =  mvp.proj * mvp.view * dynamicubo.model * vec4(inPosition, 1.0);
}