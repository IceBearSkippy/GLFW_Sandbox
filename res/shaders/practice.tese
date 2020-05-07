#version 430

//layout (quads, equal_spacing, ccw) in; -- this causes objects to wiggle/pop when moving
layout (quads, fractional_even_spacing) in;
uniform mat4 mvp_matrix;

in vec2 tcs_out[];    // texture coordinate array coming in
out vec2 tes_out;     // scalars going out one at a time

layout (binding = 0) uniform sampler2D tex_color;
layout (binding = 1) uniform sampler2D tex_height; // provide the tex_height

void main(void) {
	// map the texture coordinates onto the sub-grid specified by the incoming control points
	vec2 tc = vec2(tcs_out[0].x + (gl_TessCoord.x) / 64.0, tcs_out[0].y + (1.0 - gl_TessCoord.y) / 64.0);

	// map the tessellated grid onto the sub-grid specified by the incoming control points
	vec4 tessellatedPoint = vec4(gl_in[0].gl_Position.x + gl_TessCoord.x / 64.0, 0.0,
								 gl_in[0].gl_Position.z + gl_TessCoord.y / 64.0, 1.0);

	// add the height from the height map to the vertex:
	tessellatedPoint.y += (texture(tex_height, tc).r / 40.0);

	gl_Position = mvp_matrix * tessellatedPoint;
	tes_out = tc;
}
