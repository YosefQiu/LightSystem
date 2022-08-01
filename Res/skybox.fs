#version 330 core
//#ifdef GL_ES
precision mediump float;
//#endif
in vec3 V_Texcoord;

uniform samplerCube U_Texture;

out vec4 fragColor;
void main()
{
    fragColor = texture(U_Texture, normalize(V_Texcoord));
}
