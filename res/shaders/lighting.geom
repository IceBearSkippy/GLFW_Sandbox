#version 430

layout (triangles) in;

in vec3 varyingNormal[]; // inputs from the vertex shader
in vec3 varyingLightDir[];
in vec3 varyingVertPos[];
in vec3 varyingHalfVector[];
in vec2 textureCoords[];
in vec3 varyingTangent[];
in vec4 shadow_coord[];
in vec3 originalVertex[];

out vec3 varyingNormalG;
out vec3 varyingLightDirG;
out vec3 varyingVertPosG;
out vec3 varyingHalfVectorG;
out vec2 textureCoordsG;
out vec3 varyingTangentG;
out vec4 shadow_coordG;
out vec3 originalVertexG;


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

layout (triangle_strip, max_vertices=3) out;
layout (binding = 0) uniform sampler2D norm_map; // bind norm_map as texture
layout (binding = 1) uniform sampler2DShadow shadow_map;
layout (binding = 2) uniform samplerCube sky_map;
layout (binding = 3) uniform sampler2D tex_map;
layout (binding = 4) uniform sampler2D height_map;


void main (void) {
	// move vertices along the normal and pass through the other vertex attributes unchanged
	for (int i=0; i<3; i++) {
		gl_Position = gl_in[i].gl_Position + normalize(vec4(varyingNormal[i], 1.0)) * 0.5;

		varyingNormalG = varyingNormal[i];
		varyingLightDirG = varyingLightDir[i];
		varyingHalfVectorG = varyingHalfVector[i];
		varyingVertPosG = varyingVertPos[i];
		textureCoordsG = textureCoords[i];
		varyingTangentG = varyingTangent[i];
		shadow_coordG = shadow_coord[i];
		originalVertexG = originalVertex[i];

		EmitVertex();
	}
	EndPrimitive();
}