#version 330 core
in vec4 position;
in vec4 color;
in vec4 texcoord;
in vec4 normal;
out vec2 V_Texcoord;
out vec4 V_Color;
out vec4 V_Normal;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    V_Color = color;
    V_Normal = normal;
    V_Texcoord = texcoord.xy;
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * position;
}
