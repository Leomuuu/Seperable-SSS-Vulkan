#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

// color cavity gloss specular micronormal normal translucency 
layout(binding = 2) uniform sampler2D texSampler[7];

layout(binding = 3) uniform FragUniform {
    vec3 viewPosition;
    vec3 lightPosition;
    vec3 lightradiance;
} fragubo;

layout(location = 0) out vec4 outColor0;
layout(location = 1) out vec4 outColor1;


float PI=3.141592653589793626;
float Translucency=1;

float G_GeometrySmith(float dotNV, float dotNL, float roughness)
{
	float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float ggx2 =  dotNV / max(dotNV * (1.0 - k) + k, 0.0000001);
    float ggx1 = dotNL / max(dotNL * (1.0 - k) + k, 0.0000001);

    return ggx1 * ggx2;
}

float D_DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a2 = roughness*roughness;
    a2 *= a2;
    float NdotH2 = max(dot(N, H), 0.0);
    NdotH2 *= NdotH2;

    float nom   = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001);
}

vec3 F_FresnelSchlick(float dotHV, vec3 F0)
{
	float tmp = 1.0 - dotHV;
	float power = tmp;
	power *= power;
	power *= power;
	power *= tmp;
	return F0 + (1.0 - F0) * power;
}

// based on http://www.thetenthplanet.de/archives/1180
mat3 calculateTBN( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * -invmax, B * invmax, N );
}


void main() {

    vec3 albedo=texture(texSampler[0], fragTexCoord).rgb;
    float roughness= 1.0f-texture(texSampler[2], fragTexCoord).x;

    float cavity =texture(texSampler[1], fragTexCoord).x;
    vec3 F0=cavity*texture(texSampler[3], fragTexCoord).xyz;



	vec3 L= normalize(fragubo.lightPosition-fragPos);
	vec3 V= normalize(fragubo.viewPosition-fragPos);
	vec3 H=normalize(L+V);
	vec3 N=normalize(fragNormal);
	float dotNL= max(dot(N,L),0.0);
	float dotNV= max(dot(N,V),0.0);

    mat3 tbn=calculateTBN(N,fragPos,fragTexCoord);
    vec3 tangentSpaceNormal = texture(texSampler[5], fragTexCoord).xyz * 2.0 - 1.0;
    N = normalize(tbn * tangentSpaceNormal);

    // Cook-Torrance BRDF
	const float D = D_DistributionGGX(N, H, roughness);
	const float G = G_GeometrySmith(dotNV, dotNL, roughness);
	const vec3 F = F_FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kd = (vec3(1.0) - F);

    vec3 specularTerm= (F*D*G)/max(4*dotNL*dotNV,1e-6 );
    vec3 diffuseTerm= albedo/PI;

    vec3 result0= (kd*diffuseTerm)*dotNL*fragubo.lightradiance;
    vec3 result1= ((1-kd)*specularTerm)*dotNL*fragubo.lightradiance;

    outColor0= vec4(result0,1.0);
    outColor1= vec4(result1,1.0);
}