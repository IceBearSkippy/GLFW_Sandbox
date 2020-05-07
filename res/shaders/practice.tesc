#version 430

uniform mat4 mvp_matrix;
layout (binding = 0) uniform sampler2D tex_color;
layout (binding = 1) uniform sampler2D tex_height;
layout (vertices = 4) out;

in vec2 tc[];
out vec2 tcs_out[];

void main(void) {
	float subdivisions = 16.0;    // tunable constant based on density of detail in height map

	if (gl_InvocationID == 0) {
		vec4 p0 = mvp_matrix * gl_in[0].gl_Position;
		vec4 p1 = mvp_matrix * gl_in[1].gl_Position;
		vec4 p2 = mvp_matrix * gl_in[2].gl_Position;
		p0 = p0 / p0.w;
		p1 = p1 / p1.w;
		p2 = p2 / p2.w;
		float width = length(p2.xy - p0.xy) * subdivisions + 1.0; // perceived "width" of tess grid
		float height = length(p1.xy - p0.xy) * subdivisions + 1.0; // perceived "height" of tess grid
		gl_TessLevelOuter[0] = height;  // set tess levels based on perceived side lengths
		gl_TessLevelOuter[1] = width;
		gl_TessLevelOuter[2] = height;
		gl_TessLevelOuter[3] = width;
		gl_TessLevelInner[0] = width;
		gl_TessLevelInner[1] = height;
	}
	// forward the texture and control points to the TES
	tcs_out[gl_InvocationID] = tc[gl_InvocationID];
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	
}