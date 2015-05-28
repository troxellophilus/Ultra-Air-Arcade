#version 330

in vec3 aPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 texCoord;

void main() {
	//vec4 WVP_Pos = P * V * M * vec4(aPos, 1.0);
	//gl_Position = WVP_Pos.xyww;
	texCoord = aPos;
	gl_Position = P * V * M * vec4(aPos, 1.0);
}
