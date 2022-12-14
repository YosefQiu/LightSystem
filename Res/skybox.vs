#version 330 core

in vec4 position;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
out vec3 V_Texcoord;
void main()
{
    V_Texcoord = -position.xyz;
    gl_Position=ProjectionMatrix * ViewMatrix * ModelMatrix * position;
}
