#version 450

layout (location = 0) in vec2 uv;
// in view space
layout (location = 1) in vec3 fragPos;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 lightPos;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform sampler2D textureSampler;

void main() {
	vec4 materialColor = texture(textureSampler, uv);
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	
	vec3 lightDir = normalize(lightPos - fragPos);
	float strength = 0.8;
	float diffuse = max(dot(lightDir, normal), 0.0) * strength;

	vec3 finalColor = (diffuse * lightColor + vec3(0.6)) * materialColor.xyz;

	outColor = vec4(min(finalColor, vec3(1.0, 1.0, 1.0)), 1.0);
}
