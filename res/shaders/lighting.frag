#version 430
in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 varyingHalfVector;
in vec2 textureCoords;
in vec4 shadow_coord;
in vec3 originalVertex;

out vec4 fragColor;

struct PositionalLight
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};

struct Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;

layout (binding = 0) uniform sampler2D samp;
layout (binding = 1) uniform sampler2DShadow shTex;
layout (binding = 2) uniform samplerCube tex_map;

float lookup(float ox, float oy) {
	float f = textureProj(shTex,
		shadow_coord + vec4(ox * 0.001 * shadow_coord.w, oy * 0.001 * shadow_coord.w,
		-0.01, 0.0)); // the third parameter (-0.01) is an offset to conteract shadow acne
		return f;
}

float shadowSample(void) {
	float shadowFactor = 0.0;
	//--- this section produces a 64-sample hi-resolution soft shadow
	float swidth = 2.5;    // tunable amount of shadow spread
	float endp = swidth*3.0 + swidth/2.0;
	for (float m=-endp; m<=endp; m=m+swidth) {
		for (float n=-endp; n<=endp; n=n+swidth) {
			shadowFactor += lookup(m, n);
		}
	}
	shadowFactor = shadowFactor / 64.0;
	return shadowFactor;
}

float shadowSimpleSample(void) {
	float shadowFactor = 0.0;
	//---- this section produces a 4-sample dithered soft shadow
	float swidth = 2.5;    // tunable amount of shadow spread
	// produces one of 4 sample patterns depending on glFragCoord mod 2
	
	vec2 offset = mod(floor(gl_FragCoord.xy), 2.0) * swidth;
	shadowFactor += lookup(-1.5*swidth + offset.x, 1.5*swidth - offset.y);
	shadowFactor += lookup(-1.5*swidth + offset.x, -0.5*swidth - offset.y);
	shadowFactor += lookup(0.5*swidth + offset.x, 1.5*swidth - offset.y);
	shadowFactor += lookup(0.5*swidth + offset.x, -0.5*swidth - offset.y);
	shadowFactor = shadowFactor / 4.0;  //shadowFactor is an average of the four sampled points

	return shadowFactor;
}

vec3 generateNormalBumps(vec3 originalVertex, vec3 varyingNormal) {
	float a = 0.25; // a controls the height of bumps
	float b = 100.0; // b controls width of bumps
	float x = originalVertex.x;
	float y = originalVertex.y;
	float z = originalVertex.z;
	vec3 N;
	N.x = varyingNormal.x + a*sin(b*x);
	N.y = varyingNormal.y + a*sin(b*y);
	N.z = varyingNormal.z + a*sin(b*z);
	N = normalize(N);
	return N;
}

void main(void) {
	
	// normalize the light, normal and view vectors
	vec3 L = normalize(varyingLightDir);
	//vec3 N = normalize(varyingNormal);
	vec3 N = generateNormalBumps(originalVertex, varyingNormal);
	vec3 V = normalize(varyingVertPos);
	vec3 H = normalize(varyingHalfVector);

	

	float shadowFactor = shadowSample();
	vec4 shadowColor = globalAmbient * material.ambient * light.ambient * material.ambient;
	
	// this uses the sampler to get the color related multiply to lightedColor to add texture
	vec4 texColor = texture(samp, textureCoords);
	
	vec4 lightedColor = light.diffuse * material.diffuse * max(dot(L,N), 0.0) 
						+ light.specular  * material.specular 
						* pow(max(dot(H, N), 0.0), material.shininess * 3.0);
	fragColor = vec4((shadowColor.xyz + shadowFactor * (lightedColor.xyz)), 1.0);
	
	//fragColor += texColor; // this will enable texture

	// adds reflection layer to color (based on cubemap)
	//vec3 r = -reflect(normalize(-varyingVertPos), normalize(varyingNormal));
	//fragColor += texture(tex_map, r);
	
}



