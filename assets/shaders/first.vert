#version 450

#extension GL_EXT_buffer_reference : require

struct Vertex {
	vec3 pos;
	vec2 uv;
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

void main() {
	mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
	vec4 pos = vec4(vertexBuffer.vertices[gl_VertexIndex].pos, 1.0);
	gl_Position = mvpMatrix * pos;
	
	outUV = vertexBuffer.vertices[gl_VertexIndex].uv;
}
