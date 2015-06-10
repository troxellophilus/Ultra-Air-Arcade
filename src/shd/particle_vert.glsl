#version 330

in vec4 position;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform vec3 BillboardPos;
uniform vec2 BillboardSize;

void main() {
	vec3 particleCenter_worldspace = (vec4(BillboardPos, 1.0)).xyz;

	vec3 vertexPosition_worldspace = 
		particleCenter_worldspace
		+ CameraRight_worldspace * position.x * BillboardSize.x
		+ CameraUp_worldspace * position.y * BillboardSize.y;

	gl_Position = P * V * vec4(vertexPosition_worldspace, 1.0f);
}
