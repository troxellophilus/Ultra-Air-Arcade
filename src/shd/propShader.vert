#version 330

layout (location = 0) in vec4 prop_position;
layout (location = 1) in vec3 prop_normal;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform vec3 diffuse;

out vec3 vNor;
out vec3 vPos;
out vec3 vCol;

void main() 
{
	vPos = vec3(M * prop_position);
	vNor = vec3(M * vec4(prop_normal, 0.0));
	vCol = diffuse;
	gl_Position = P * V * M * prop_position;
}
