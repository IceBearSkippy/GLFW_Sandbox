#version 430

in vec3 tc; // interpolated incoming texture coordinate
out vec4 fragColor;

uniform mat4 v_matrix;
uniform mat4 proj_matrix;

layout (binding=0) uniform samplerCube samp;

void main(void) { 

	fragColor = texture(samp, tc);

}