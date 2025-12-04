#version 450

#extension GL_EXT_buffer_reference : require

struct Vertex {
	vec3 pos;
	float u;
	vec3 normal;
	float v;
};

layout(buffer_reference, std430) buffer readonly VertexBuffer {
	Vertex vertices[];
};

layout (std430, push_constant) uniform Constants {
	VertexBuffer vertexBuffer;
	mat4 modelMatrix;
};

layout (std140, set = 0, binding = 0) uniform UBO {
	mat4 viewMatrix; 
	mat4 projectionMatrix; 
};

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outViewPos;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outLightPos;

void main() {
	Vertex vertex = vertexBuffer.vertices[gl_VertexIndex];
	vec2 uv = vec2(vertex.u, vertex.v);

	mat4 modelViewMatrix = viewMatrix * modelMatrix;
	
	vec4 viewPos = modelViewMatrix * vec4(vertex.pos, 1.0);
	vec3 lightPos = vec3(0.0, 5.0, 3.0);
	
	gl_Position = projectionMatrix * viewPos;
	
	outUV = uv;
	outViewPos = (modelViewMatrix * vec4(vertex.pos, 1.0)).xyz;
	outNormal = normalize(modelViewMatrix * vec4(normalize(vertex.normal), 0.0)).xyz;
	outLightPos = (viewMatrix * vec4(lightPos, 1.0)).xyz;
}
