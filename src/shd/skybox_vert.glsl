#version 330

in vec3 aPos;

uniform mat4 P;
uniform mat4 V;

out vec3 texCoord;

void main() {
	vec4 WVP = P * V * vec4(aPos, 1.0);
	gl_Position = WVP.xyww;
	texCoord = aPos;
}
