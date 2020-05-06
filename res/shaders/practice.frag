#version 430
in vec2 texCoord_TESout;
out vec4 color;
uniform mat4 mvp_matrix;
layout (binding = 0) uniform sampler2D tex_color;
void main(void) {
	color = vec4(1.0, 1.0, 0.0, 1.0);
	//color = texture(tex_color, texCoord_TESout);
}