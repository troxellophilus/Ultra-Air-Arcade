#version 330

in vec4 aPos;
in vec3 aNor;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec3 vNor;
out vec3 vPos;
out vec3 vCol;

void main() 
{
	vPos = vec3(M * aPos);
	vNor = vec3(M * vec4(aNor, 0.0));
	vCol = vec3(1.0f,0.0f,0.0f);
	gl_Position = P * V * M * aPos;
}
