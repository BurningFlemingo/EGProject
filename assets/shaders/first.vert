#version 450
#extension GL_EXT_buffer_reference : require

layout(buffer_reference, std430) buffer readonly VertexBuffer {
	vec4 positions[];
};

layout (std430, push_constant) uniform constants {
	VertexBuffer vertexBuffer;
	mat4 mvp;
} pushConstants;

layout (std140, set = 0, binding = 0) uniform UBO {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} ubo;


layout (location = 0) out vec3 outFragColor;

void main() {
	vec4 pos = pushConstants.vertexBuffer.positions[gl_VertexIndex];
	gl_Position = pushConstants.mvp * pos;
	
	outFragColor = vec3(0.5, 0.5, 1.0);
}
