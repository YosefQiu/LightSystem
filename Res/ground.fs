#version 330 core
#ifdef GL_ES
precision mediump float;
#endif
uniform vec4 U_LightPos;
uniform vec4 U_LightAmbient;
uniform vec4 U_LightDiffuse;
uniform vec4 U_AmbientMaterial;
uniform vec4 U_DiffuseMaterial;
in vec4 V_Color;
in vec4 V_Normal;
in vec4 V_WorldPos;

out vec4 FragColor;
void main()
{
    vec4 color = vec4(0.0,0.0,0.0,0.0);
    vec4 ambientColor = U_LightAmbient * U_AmbientMaterial;
    vec3 lightPos = U_LightPos.xyz;
    vec3 L = U_LightPos.xyz - V_WorldPos.xyz;
    float dist = length(L);
    float constantFactor = 1.0;
    float linearFactor = 0.0;
    float quadricFactor = 0.0;
    float attenuation = 1.0 / (constantFactor + linearFactor * dist + quadricFactor * quadricFactor * dist);
    L = normalize(L);
    vec3 N = normalize(V_Normal.xyz);
    float diffuseIntensity = max(0.0,dot(L, N));
    vec4 diffuseColor = U_LightDiffuse * U_DiffuseMaterial * diffuseIntensity * attenuation;
    
    color = ambientColor + diffuseColor;
    
    FragColor = color * V_Color;
}
