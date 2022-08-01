#version 330 core

in vec2 V_Texcoord;
uniform sampler2D gPos;
uniform sampler2D gNor;
uniform sampler2D texNoise;
uniform vec3 samples[64];
uniform mat4 projMatrix;

out vec4 FragColor;

int kernelSize = 64;
float radius = 1.0f;

const vec2 noiseScale = vec2(1280.0f / 4.0f, 720.0f / 4.0f);
vec2 CalcTexCoord() {
	return gl_FragCoord.xy / vec2(1280.0f, 720.0f);
}

void main() {
	vec3 worldPos = texture(gPos, V_Texcoord.xy).rgb;
	vec3 N = texture(gNor, V_Texcoord.xy).xyz;
	vec3 randomVec = texture(texNoise, V_Texcoord * noiseScale).xyz;

	vec3 tangent = normalize(randomVec - N * dot(randomVec, N));
	vec3 bitangent = cross(N, tangent);
	mat3 TBN = mat3(tangent, bitangent, N);
	
	float occlusion = 0.0f;
}