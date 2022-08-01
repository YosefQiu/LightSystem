#version 330 core

in vec2 V_Texcoord;
uniform sampler2D U_Texture;
uniform vec4 lightPos;

out vec4 FragColor;

uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform float samples;

void main() {
    vec3 col = texture(U_Texture, V_Texcoord.xy).rgb;
    vec2 deltaTextCoord = V_Texcoord.xy - lightPos.xy;
    vec2 textCord = V_Texcoord.xy;
    deltaTextCoord *= 1.0f / samples * density;

    float illuminationDecay = 1.0f;
    for (int i = 0; i < samples; i++) {
        textCord -= deltaTextCoord;
        vec3 sampled_color = texture(U_Texture, textCord).rgb;
        sampled_color *= illuminationDecay * weight;
        col += sampled_color;
        illuminationDecay *= decay;
    }

    col *= exposure;
    FragColor = vec4(col, 1.0);
}