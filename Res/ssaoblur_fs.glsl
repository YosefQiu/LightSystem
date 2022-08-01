#version 330 core

in vec2 V_Texcoord;
uniform sampler2D U_Texture;
uniform bool bDepth;

out vec4 FragColor;

float LinearizeDepth(float depth) {
	float near = 0.1f;
	float far = 100.0f;
	float z = depth * 2.0f - 1.0f;
	return (2.0f * near * far) / (far + near - z * (far - near));
}

void main() {
	const float gamma = 2.2f;
	vec3 hdrColor = vec3(0.0f);
	vec3 mapped = vec3(0.0f);
	if (bDepth) {
		float depthValue = texture(U_Texture, V_Texcoord.xy).r;
		depthValue = LinearizeDepth(depthValue) / 100.0f;
		mapped = vec3(depthValue);
	}
	else {
		hdrColor = texture(U_Texture, V_Texcoord.xy).rgb;
		mapped = hdrColor / (hdrColor + vec3(1.0f));
		mapped = pow(mapped, vec3(1.0f / gamma));
	}
	
	FragColor = vec4(mapped, 1.0f);
}