#version 330
in vec2 position;
in vec2 texcoord;

out vec2 UV;

uniform mat4 P;
uniform mat4 V;
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform vec3 BillboardPos;
uniform vec2 BillboardSize;

void main() {
	vec3 particleCenter_wordspace = BillboardPos;

	vec3 vertexPosition_worldspace = 
		particleCenter_wordspace
		+ CameraRight_worldspace * position.x * BillboardSize.x
		+ CameraUp_worldspace * position.y * BillboardSize.y;

	gl_Position = P * V * vec4(vertexPosition_worldspace, 1.0f);
	UV = texcoord;
}
