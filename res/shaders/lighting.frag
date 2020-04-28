#version 430
in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 varyingHalfVector;
in vec2 textureCoords;
in vec4 shadow_coord;
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

void main(void) {
	// normalize the light, normal and view vectors
	vec3 L = normalize(varyingLightDir);
	vec3 N = normalize(varyingNormal);
	vec3 V = normalize(varyingVertPos);
	vec3 H = normalize(varyingHalfVector);

	float notInShadow = textureProj(shTex, shadow_coord);
	
	vec4 texColor = texture(samp, textureCoords); // this uses the sampler to get the color related -- Note, this ain't needed unless texture is available
	
	//fragColor = texColor * globalAmbient * material.ambient + light.ambient * material.ambient;
	fragColor = globalAmbient * material.ambient + light.ambient * material.ambient;
	if (notInShadow == 1.0) {
		fragColor += light.diffuse * material.diffuse * max(dot(L,N), 0.0) + light.specular  * material.specular * pow(max(dot(H, N), 0.0), material.shininess * 3.0);
	}
	
}