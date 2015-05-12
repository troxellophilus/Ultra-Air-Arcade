#version 330

in vec3 position;
in vec3 texCoord;
in vec3 normal;

uniform mat4 gWVP;

out vec2 texCoordOut;

void main() {
	gl_Position = gWV{ * vec4(position, 1.0);
	texCoordOut = texCoord;
}
