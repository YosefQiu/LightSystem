#version 330 core

in vec2 V_Texcoord;
in vec4 V_Color;
in vec4 V_Normal;
in vec4 V_WorldPos;

out vec4 FragColor;

void main()
{
	FragColor = vec4(vec3(0.0f), 1.0f);
}
