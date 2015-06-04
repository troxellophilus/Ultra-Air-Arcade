#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 aPos;
in vec3 aNor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lPos;
uniform mat4 depthBiasMVP;

// Output data ; will be interpolated for each fragment.
out vec3 vPos;
out vec3 vNor;
out vec3 eye;
out vec3 lightDirection;
out vec3 lightPosition;
out vec4 shadowCoord;
out vec3 silh_vNor;
out vec3 silh_vPos;

void main(){
	gl_Position =  P * V * M * vec4(aPos.xyz,1.0);
	
	shadowCoord = depthBiasMVP * vec4(aPos, 1.0);
	lightPosition = lPos;
	eye = vec3(0,0,0) - ( V * M * vec4(aPos, 1)).xyz;
	lightDirection = (vec4(lPos, 0.0)).xyz;

	vPos = (M * vec4(aPos, 1.0)).xyz;
    vNor = ( M * vec4(aNor, 0.0)).xyz;

	silh_vPos = vec3((V * M * vec4(aPos.xyz, 1.0)).xyz);
	silh_vNor = vec3(V * M * vec4(aNor, 0.0));
}

