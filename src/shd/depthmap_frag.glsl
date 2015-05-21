#version 330 core

// Ouput data
out float fragmentDepth;

void main() {
		fragmentDepth = gl_FragCoord.z;
}
