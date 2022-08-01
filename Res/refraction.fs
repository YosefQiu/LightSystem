#version 330 core
#ifdef GL_ES
precision mediump float;
#endif
in vec3 V_Texcoord;
in vec4 V_WorldPos;
in vec4 V_Normal;

uniform samplerCube U_Texture;
uniform vec4 U_CameraPos;

out vec4 fragColor;
void main()
{
    vec3 eyeVec = normalize(V_WorldPos.xyz - U_CameraPos.xyz);
    vec3 N = normalize(V_Normal.xyz);
    vec3 R = refract(eyeVec, N, 1.0 / 1.52);
    fragColor = texture(U_Texture, R);
}
