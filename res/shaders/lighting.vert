#version 430
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 vertNormal;
layout (location = 3) in vec3 vertTangent;

out vec3 varyingNormal;    // eye-space vertex normal
out vec3 varyingLightDir;  // vector pointing to the light
out vec3 varyingVertPos;   // vertex position in eye space
out vec3 varyingHalfVector;
out vec2 textureCoords;
out vec3 varyingTangent;
out vec4 shadow_coord;
out vec3 originalVertex;

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
uniform mat4 norm_matrix; // for transforming normals
uniform mat4 shadowMVP;
uniform int enableLighting;

layout (binding = 0) uniform sampler2D norm_map; // bind norm_map as texture
layout (binding = 1) uniform sampler2DShadow shadow_map;
layout (binding = 2) uniform samplerCube sky_map;
layout (binding = 3) uniform sampler2D tex_map;
layout (binding = 4) uniform sampler2D height_map;

void main(void) {
	// "p" is the vertex position altered by the height map
	// Since the height map is grayscale, any of the color components can be
	// used (we use "r"). Dividing by 5.0 is to adjust the height
	//vec4 p = vec4(vertPos, 1.0) + vec4((vertNormal * ((texture(height_map, texCoord).r) / 5.0)), 1.0);
	vec4 p = vec4((vertNormal * ((texture(height_map, texCoord).r) / 5.0)), 1.0);

	// keep the original vertices
	originalVertex = vertPos;

	// output vertex position, light direction and normal to the rasterizer for interpolation
	varyingVertPos = (mv_matrix * vec4(vertPos, 1.0)).xyz;
	varyingLightDir = light.position - varyingVertPos;
	varyingNormal = (norm_matrix * vec4(vertNormal, 1.0)).xyz;
	varyingTangent = (norm_matrix * vec4(vertTangent, 1.0)).xyz;

	varyingHalfVector = (varyingLightDir + (-varyingVertPos)).xyz;
	shadow_coord = shadowMVP * vec4(vertPos, 1.0);
	textureCoords = texCoord;
	
	gl_Position = proj_matrix * mv_matrix * vec4(vertPos, 1.0) + p;

}