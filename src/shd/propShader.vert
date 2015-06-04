#version 330

in vec4 prop_position;
in vec3 prop_normal;
in vec2 vertTex;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec3 vNor;
out vec3 vPos;
out vec3 vCol;
out vec2 fragTex;

void main() 
{
	vPos = vec3(M * prop_position);
	vNor = vec3(M * vec4(prop_normal, 0.0));
	vCol = vec3(0.5f,1.0f,0.5f);
	fragTex = vertTex;
	gl_Position = P * V * M * prop_position;
}
