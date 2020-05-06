#version 430
out vec2 texCoord;
uniform mat4 mvp_matrix;
layout (binding = 0) uniform sampler2D tex_color;

void main(void) {
	// this time the vertex shader defines and sends out control points:
	//const vec4 vertices[] =
	//vec4[] (vec4(-1.0, 0.5, -1.0, 1.0), vec4(-0.5, 0.5, -1.0, 1.0),
	//		vec4(0.5, 0.5, -1.0, 1.0), vec4(1.0, 0.5, -1.0, 1.0),

	//		vec4(-1.0, 0.0, -0.5, 1.0), vec4(-0.5, 0.0, -0.5, 1.0),
	//		vec4(0.5, 0.0, -0.5, 1.0), vec4(1.0, 0.0, -0.5, 1.0),

	//		vec4(-1.0, 0.0, 0.5, 1.0), vec4(-0.5, 0.0, 0.5, 1.0),
	//		vec4(0.5, 0.0, 0.5, 1.0), vec4(1.0, 0.0, 0.5, 1.0),

	//		vec4(-1.0, -0.5, 1.0, 1.0), vec4(-0.5, 0.3, 1.0, 1.0),
	//		vec4(0.5, 0.3, 1.0, 1.0), vec4(1.0, 0.3, 1.0, 1.0));

	// This is a square locked to y axis
	const vec4 vertices[] =
	vec4[] (vec4(-1.0, 0.0, -1.0, 1.0), vec4(-0.5, 0.0, -1.0, 1.0),
			vec4(0.5, 0.0, -1.0, 1.0), vec4(1.0, 0.0, -1.0, 1.0),

			vec4(-1.0, 0.0, -0.5, 1.0), vec4(-0.5, 0.0, -0.5, 1.0),
			vec4(0.5, 0.0, -0.5, 1.0), vec4(1.0, 0.0, -0.5, 1.0),

			vec4(-1.0, 0.0, 0.5, 1.0), vec4(-0.5, 0.0, 0.5, 1.0),
			vec4(0.5, 0.0, 0.5, 1.0), vec4(1.0, 0.0, 0.5, 1.0),

			vec4(-1.0, 0.0, 1.0, 1.0), vec4(-0.5, 0.0, 1.0, 1.0),
			vec4(0.5, 0.0 , 1.0, 1.0), vec4(1.0, 0.0, 1.0, 1.0));

	//compute an appropriate texture coordinate for the current vertex, shifted from [-1..+1] to [0..1]
	texCoord = vec2((vertices[gl_VertexID].x + 1.0) / 2.0, (vertices[gl_VertexID].z + 1.0) / 2.0);
	gl_Position = vertices[gl_VertexID];
}