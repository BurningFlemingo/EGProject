#version 450
#extension GL_EXT_buffer_reference : require

struct Vertex{
	vec4 pos; 
	vec4 color; 
};

layout(buffer_reference, std430) buffer readonly VertexBuffer {
	Vertex vertices[];
};

layout (push_constant) uniform constants {
	VertexBuffer vertexBuffer;
} pushConstants;


layout (location = 0) out vec3 outFragColor;

void main() {
	Vertex vertex = pushConstants.vertexBuffer.vertices[gl_VertexIndex];
	gl_Position = vertex.pos;
	
	outFragColor = vertex.color.xyz;
}
