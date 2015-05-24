#version 330

in vec3 texCoord;

out vec4 outColor;

uniform samplerCube cubeMapTexture;

void main() {
	outColor = texture(cubeMapTexture, texCoord);
}
