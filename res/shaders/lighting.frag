#version 430
in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 varyingHalfVector;
in vec3 varyingTangent;
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

layout (binding = 0) uniform sampler2D norm_map;
layout (binding = 1) uniform sampler2DShadow shadow_map;
layout (binding = 2) uniform samplerCube sky_map;
layout (binding = 3) uniform sampler2D tex_map;


float lookup(float ox, float oy) {
	float f = textureProj(shadow_map,
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

vec3 calcNewNormal() {
	vec3 normal = normalize(varyingNormal);
	vec3 tangent = normalize(varyingTangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);  // tangent is perpendicular to normal
	vec3 bitangent = cross(tangent, normal);
	mat3 tbn = mat3(tangent, bitangent, normal);  // TBN matrix to convert to camera space
	vec3 retrievedNormal = texture(norm_map, textureCoords).xyz;
	retrievedNormal = retrievedNormal * 2.0 - 1.0; 
	vec3 newNormal = tbn * retrievedNormal;
	newNormal = normalize(newNormal);
	return newNormal;
}

void main(void) {
	
	// normalize the light, normal and view vectors
	vec3 L = normalize(varyingLightDir);
	//vec3 N = normalize(varyingNormal);
	//vec3 N = generateNormalBumps(originalVertex, varyingNormal);
	vec3 N = calcNewNormal();
	vec3 V = normalize(varyingVertPos);
	vec3 H = normalize(varyingHalfVector);

	float cosTheta = dot(L,N);
	float cosPhi = dot(H,N);

	float shadowFactor = shadowSample();
	vec4 shadowColor = globalAmbient * material.ambient * light.ambient * material.ambient;
	// this uses the sampler to get the color related multiply to lightedColor to add texture
	vec4 texel = texture(tex_map, textureCoords);
	
	vec4 lightedColor = texel * (light.diffuse * material.diffuse * max(cosTheta, 0.0) 
						+ light.specular  * material.specular 
						* pow(max(cosPhi, 0.0), material.shininess));
	fragColor =  vec4((shadowColor.xyz + shadowFactor * (lightedColor.xyz)), 1.0);
	
	//fragColor += texel; // this will enable texture

	// adds reflection layer to color (based on cubemap)
	//vec3 r = -reflect(normalize(-varyingVertPos), N);
	//fragColor += texture(sky_map, r);
	
}



