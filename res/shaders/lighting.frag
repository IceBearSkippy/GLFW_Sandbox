#version 430
in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 varyingHalfVector;
in vec2 textureCoords;
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

layout (binding=0) uniform sampler2D samp;

void main(void) {
	// normalize the light, normal and view vectors
	vec3 L = normalize(varyingLightDir);
	vec3 N = normalize(varyingNormal);
	vec3 V = normalize(varyingVertPos);
	vec3 H = normalize(varyingHalfVector);

	// get the angle between the light and surface normal
	float cosTheta = dot(L,N);
	// get angle between the normal and the halfway vector
	float cosPhi = dot(H,N);

	// compute ADS contributions (per pixel), and combine to build output color
	vec3 ambient = ((globalAmbient * material.ambient) + (light.ambient * material.ambient)).xyz;
	vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(cosTheta, 0.0);

	// the multiplaction by 3.0 at the end is a tweak to improve the specular highlight
	vec3 specular = light.specular.xyz * material.specular.xyz * pow(max(cosPhi, 0.0), material.shininess*3.0);
	
	vec4 texColor = texture(samp, textureCoords); // this uses the sampler to get the color related -- Note, this ain't needed unless texture is available
	
	//fragColor = vec4((ambient + diffuse + specular), 1.0); // without texture

	fragColor = texColor * vec4((ambient + diffuse ), 1.0) + vec4(specular, 1.0);
	
}