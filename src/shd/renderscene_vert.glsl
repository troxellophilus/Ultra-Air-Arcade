#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 aPos;
in vec3 aNor;

// Output data ; will be interpolated for each fragment.
out vec3 position_worldspace;
out vec3 normal_cameraspace;
out vec3 eye;
out vec3 lightDirection_cameraspace;
out vec4 shadowCoord;

// Values that stay constant for the whole mesh.
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lPos;
uniform mat4 depthBiasMVP;


void main(){
	gl_Position =  P * V * M * vec4(aPos,1.0);
	
	shadowCoord = depthBiasMVP * vec4(aPos, 1);
	position_worldspace = (M * vec4(aPos, 1)).xyz;
	
	eye = vec3(0,0,0) - ( V * M * vec4(aPos, 1)).xyz;

	lightDirection_cameraspace = (V*vec4(lPos, 0)).xyz;
	
	normal_cameraspace = ( V * M * vec4(aNor, 0)).xyz;
}

