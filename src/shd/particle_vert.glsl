#version 330

in vec3 position;

uniform mat4 P;
uniform mat4 V;
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform vec3 particlePos;
uniform vec2 particleSize;
uniform int timeStep;

out float time;

float resize() {
	return 0.025 - (0.014 * timeStep /60.0);
}

void main() {
	vec3 particleCenter_worldspace = particlePos;

	float scale = resize();
	vec3 vertexPosition_worldspace = 
		particleCenter_worldspace
		+ CameraRight_worldspace * position.x * scale
		+ CameraUp_worldspace * position.y * scale;
	
	gl_Position = P * V * vec4(vertexPosition_worldspace, 1.0f);
	time = timeStep;
}
