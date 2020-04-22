#version 430

layout (location=0) in vec3 position;
layout (location=1) in vec2 texCoord;
out vec2 tc; //texture coordinate output to rasterize for interpolation
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
layout (binding = 0) uniform sampler2D samp; // not used in vertex shader


mat4 buildRotateX(float rad);
mat4 buildRotateY(float rad);
mat4 buildRotateZ(float rad);
mat4 buildTranslate(float x, float y, float z);

void main(void) 
{   
    //mat4 mv_matrix = v_matrix * m_matrix;

    gl_Position = proj_matrix * mv_matrix * vec4(position, 1.0);
	tc = texCoord;
}


//cant import utilities in GLSL
// only use if requiring instance of shader rotation
mat4 buildRotateX(float rad) {
    mat4 xrot = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, cos(rad), -sin(rad), 0.0,
        0.0, sin(rad), cos(rad), 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    return xrot;
}

mat4 buildRotateY(float rad) {
    mat4 yrot = mat4(
        cos(rad), 0.0, sin(rad), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(rad), 0.0, cos(rad), 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    return yrot;
}

mat4 buildRotateZ(float rad) {
    mat4 zrot = mat4(
        cos(rad), -sin(rad), 0.0, 0.0,
        sin(rad), cos(rad), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    return zrot;
}

mat4 buildTranslate(float x, float y, float z) {
    mat4 trans = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        x, y, z, 1.0
    );
    return trans;
}