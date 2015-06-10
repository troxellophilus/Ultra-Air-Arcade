#version 330

in vec3 position;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform vec3 BillboardPos;
uniform vec2 BillboardSize;

void main() {
	vec3 particleCenter_worldspace = BillboardPos;

	vec3 vertexPosition_worldspace = 
		particleCenter_worldspace
		+ CameraRight_worldspace * position.x * BillboardSize.x
		+ CameraUp_worldspace * position.y * BillboardSize.y;
	
	mat4 matA = mat4( 1.0, 0.0, 0.0, 0.0,
						0.0, 1.0, 0.0, 0.0,
						0.0, 0.0, 1.0, -0.25,
						0.0, 0.0, 0.0, 1.0 );
	mat4 matB = mat4( 1.0, 0.0, 0.0, vertexPosition_worldspace.x,
						0.0, 1.0, 0.0, vertexPosition_worldspace.y,
						0.0, 0.0, 1.0, vertexPosition_worldspace.z,
						0.0, 0.0, 0.0, 1.0 );
	
	//mat4 matB = mat4(1.0) * vec4(vertexPosition_worldspace, 1.0f);
	gl_Position = P * V * vec4(vertexPosition_worldspace, 1.0f);
}
