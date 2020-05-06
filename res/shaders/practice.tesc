#version 430
in vec2 texCoord[];        // The texture coords output from the vertex shader as scalars arrive
out vec2 texCoord_TCSout[]; // in an array and are then passed through to the evaluation shader

uniform mat4 mvp_matrix;
layout (binding = 0) uniform sampler2D tex_color;
layout (vertices = 16) out;    // there are 16 control points per patch

void main(void) {
	int TL = 32; // tesselation levels are all set to this value
	if (gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = TL;
		gl_TessLevelOuter[1] = TL;
		gl_TessLevelOuter[2] = TL;
		gl_TessLevelOuter[3] = TL;
		gl_TessLevelInner[0] = TL;
		gl_TessLevelInner[1] = TL;
	}
	// forward the texture and control points to the TES
	texCoord_TCSout[gl_InvocationID] = texCoord[gl_InvocationID];
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	
}