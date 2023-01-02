#version 450

layout(binding = 0) uniform MVPMatrix {
    mat4 view;
    mat4 proj;
} mvp;

layout(binding = 1) uniform dynamicUbo{
    mat4 model;
}dynamicubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragTexCoord;


void main() {
    gl_Position = mvp.proj * mvp.view * dynamicubo.model * vec4(inPosition , 1.0);
    fragPos = (dynamicubo.model *vec4(inPosition,1.0)).xyz;
    fragColor = inColor;
    fragNormal = mat3(dynamicubo.model) *inNormal;
    fragTexCoord = inTexCoord;
}