#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(binding = 2) uniform sampler2D texSampler;

layout(binding = 3) uniform FragUniform {
    vec3 viewPosition;
    vec3 lightPosition;
} fragubo;

layout(location = 0) out vec4 outColor;

void main() {
    
    vec3 color = texture(texSampler, fragTexCoord).rgb;

    // ambient
    vec3 ambient = 0.05 * color;
    // diffuse
    vec3 lightDir = normalize(fragubo.lightPosition - fragPos);
    vec3 normal = normalize(fragNormal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
     // specular
    vec3 viewDir = normalize( fragubo.viewPosition - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.3) * spec; 

    vec4 FragColor = vec4(ambient+diffuse+specular, 1.0);

    outColor = FragColor;
}