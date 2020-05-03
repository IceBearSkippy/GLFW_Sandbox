#version 430

layout (location=0) in vec3 position;

out vec3 tc; //texture coordinate output to rasterize for interpolation
uniform mat4 v_matrix;
uniform mat4 proj_matrix;
layout (binding = 0) uniform samplerCube samp; // for cube mapping


void main(void) 
{   
    tc = position;  // texture coordinates are simply the vertex coordinates
    mat4 vrot_matrix = mat4(mat3(v_matrix)); // removes translation from view matrix
    gl_Position = proj_matrix * vrot_matrix * vec4(position, 1.0);
}
