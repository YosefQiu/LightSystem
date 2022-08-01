#version 330 core
in vec4 position;
in vec4 texcoord;

out vec2 V_Texcoord;

void main() {
	V_Texcoord = texcoord.xy;
	gl_Position = position;
}