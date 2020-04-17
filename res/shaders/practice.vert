#version 430

uniform float offset;
uniform mat4 transformation;
void main(void) 
{
	if (gl_VertexID == 0) {
		gl_Position = transformation * vec4(0.25, -0.25, 0.0, 1.0);
	} else if (gl_VertexID == 1) {
		gl_Position = transformation * vec4(-0.25, -0.25, 0.0, 1.0);
	} else {
		gl_Position = transformation * vec4(0.25, 0.25, 0.0, 1.0);
	}

}