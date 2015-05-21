#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 aPos;

// Values that stay constant for the whole mesh.
uniform mat4 depthMVP;

void main(){
	gl_Position =  depthMVP * vec4(aPos, 1);
}

