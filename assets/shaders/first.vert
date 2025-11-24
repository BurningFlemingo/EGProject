#version 450
#extension GL_EXT_buffer_reference : require

layout(buffer_reference, std430) buffer readonly VertexBuffer {
	vec4 positions[];
};

layout (std430, push_constant) uniform constants {
	VertexBuffer vertexBuffer;
	mat4 mvp;
} pushConstants;


layout (location = 0) out vec3 outFragColor;

void main() {
	uint meshVertexIndex = gl_VertexIndex;
	vec4 pos = pushConstants.vertexBuffer.positions[meshVertexIndex];
	gl_Position = pushConstants.mvp * pos;
	
	outFragColor = vec3(0.5, 0.5, 1.0);
}
