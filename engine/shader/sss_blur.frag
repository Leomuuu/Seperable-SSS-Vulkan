#version 450

layout(binding = 2) uniform sampler2D uInputTexture;
layout(binding = 3) uniform sampler2D uDepthTexture;

layout(push_constant) uniform PushConsts {
	vec2 dir;
	vec2 windowSize;
	float sssWidth;
} pushConsts;

layout(location = 0) out vec4 outColor;

vec4 kernel25[] = 
{
	vec4(0.530605, 0.613514, 0.739601, 0),
	vec4(0.000973794, 1.11862e-005, 9.43437e-007, -3),
	vec4(0.00333804, 7.85443e-005, 1.2945e-005, -2.52083),
	vec4(0.00500364, 0.00020094, 5.28848e-005, -2.08333),
	vec4(0.00700976, 0.00049366, 0.000151938, -1.6875),
	vec4(0.0094389, 0.00139119, 0.000416598, -1.33333),
	vec4(0.0128496, 0.00356329, 0.00132016, -1.02083),
	vec4(0.017924, 0.00711691, 0.00347194, -0.75),
	vec4(0.0263642, 0.0119715, 0.00684598, -0.520833),
	vec4(0.0410172, 0.0199899, 0.0118481, -0.333333),
	vec4(0.0493588, 0.0367726, 0.0219485, -0.1875),
	vec4(0.0402784, 0.0657244, 0.04631, -0.0833333),
	vec4(0.0211412, 0.0459286, 0.0378196, -0.0208333),
	vec4(0.0211412, 0.0459286, 0.0378196, 0.0208333),
	vec4(0.0402784, 0.0657244, 0.04631, 0.0833333),
	vec4(0.0493588, 0.0367726, 0.0219485, 0.1875),
	vec4(0.0410172, 0.0199899, 0.0118481, 0.333333),
	vec4(0.0263642, 0.0119715, 0.00684598, 0.520833),
	vec4(0.017924, 0.00711691, 0.00347194, 0.75),
	vec4(0.0128496, 0.00356329, 0.00132016, 1.02083),
	vec4(0.0094389, 0.00139119, 0.000416598, 1.33333),
	vec4(0.00700976, 0.00049366, 0.000151938, 1.6875),
	vec4(0.00500364, 0.00020094, 5.28848e-005, 2.08333),
	vec4(0.00333804, 7.85443e-005, 1.2945e-005, 2.52083),
	vec4(0.000973794, 1.11862e-005, 9.43437e-007, 3),
};

vec4 kernel17[] = {
    vec4(0.536343, 0.624624, 0.748867, 0),
    vec4(0.00317394, 0.000134823, 3.77269e-005, -2),
    vec4(0.0100386, 0.000914679, 0.000275702, -1.53125),
    vec4(0.0144609, 0.00317269, 0.00106399, -1.125),
    vec4(0.0216301, 0.00794618, 0.00376991, -0.78125),
    vec4(0.0347317, 0.0151085, 0.00871983, -0.5),
    vec4(0.0571056, 0.0287432, 0.0172844, -0.28125),
    vec4(0.0582416, 0.0659959, 0.0411329, -0.125),
    vec4(0.0324462, 0.0656718, 0.0532821, -0.03125),
    vec4(0.0324462, 0.0656718, 0.0532821, 0.03125),
    vec4(0.0582416, 0.0659959, 0.0411329, 0.125),
    vec4(0.0571056, 0.0287432, 0.0172844, 0.28125),
    vec4(0.0347317, 0.0151085, 0.00871983, 0.5),
    vec4(0.0216301, 0.00794618, 0.00376991, 0.78125),
    vec4(0.0144609, 0.00317269, 0.00106399, 1.125),
    vec4(0.0100386, 0.000914679, 0.000275702, 1.53125),
    vec4(0.00317394, 0.000134823, 3.77269e-005, 2),
};


vec4 kernel11[] = {
    vec4(0.560479, 0.669086, 0.784728, 0),
    vec4(0.00471691, 0.000184771, 5.07566e-005, -2),
    vec4(0.0192831, 0.00282018, 0.00084214, -1.28),
    vec4(0.03639, 0.0130999, 0.00643685, -0.72),
    vec4(0.0821904, 0.0358608, 0.0209261, -0.32),
    vec4(0.0771802, 0.113491, 0.0793803, -0.08),
    vec4(0.0771802, 0.113491, 0.0793803, 0.08),
    vec4(0.0821904, 0.0358608, 0.0209261, 0.32),
    vec4(0.03639, 0.0130999, 0.00643685, 0.72),
    vec4(0.0192831, 0.00282018, 0.00084214, 1.28),
    vec4(0.00471691, 0.000184771, 5.07565e-005, 2),
};

float linearizeDepth(float z)
{
	float n = 0.01;
	float f = 50.0;
	return (n * f) / (f - z * (f - n));
}

void main() 
{
	vec2 lightCoord=vec2(gl_FragCoord.x/pushConsts.windowSize.x,gl_FragCoord.y/pushConsts.windowSize.y);

	vec4 colorM = vec4(texture(uInputTexture,lightCoord.xy).xyz,1.0);

	// skip blurring for non SSS pixels
	if (colorM.a == 0.0)
	{
		outColor=colorM;
		return;
	}
	
	float depthM = linearizeDepth(texture(uDepthTexture,lightCoord.xy).x);
	
	float rayRadiusUV = 0.5 * pushConsts.sssWidth / depthM;

	// calculate the final step to fetch the surrounding pixels:
	vec2 finalStep = rayRadiusUV * pushConsts.dir;
	finalStep *= colorM.a;
	finalStep *= 1.0 / 3.0; // divide by 3 as the kernels range from -3 to 3
	
	// accumulate the center sample:
	vec4 colorBlurred = colorM;
	colorBlurred.rgb *= kernel25[0].rgb;
	
	
	// accumulate the other samples:
	for (int i = 1; i < 25; ++i)
	{
		// fetch color and depth for current sample:
		vec2 offset = lightCoord + kernel25[i].a * finalStep;
		vec4 color = textureLod(uInputTexture, offset, 0.0);
		float depth = linearizeDepth(textureLod(uDepthTexture, offset, 0.0).x);
		
		// lerp back to center sample if depth difference too big
		float maxDepthDiff = 0.01;
		float alpha = min(distance(depth, depthM) / maxDepthDiff, maxDepthDiff);
		
		// reject sample if it isnt tagged as SSS
		alpha *= 1.0 - color.a;
		
		color.rgb = mix(color.rgb, colorM.rgb, alpha);
		
		// accumulate:
		colorBlurred.rgb += kernel25[i].rgb * color.rgb;
	}

	outColor=colorBlurred;
	
	
}