#version 330
in vec2 UV;
out vec4 outColor;

uniform sampler2D myTextureSampler;

void main() {
	//outColor = texture2D(myTextureSampler, UV);// * vec4(1.0);
	outColor = vec4(1.0);
}
