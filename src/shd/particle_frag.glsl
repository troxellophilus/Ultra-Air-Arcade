#version 330

in float time;

out vec4 outColor;

void main() {
	vec4 startColor = vec4(0.909, 0.431, 0.156, 1.0);
	vec4 endColor = vec4(1.0, 0.988, 0.0509, 0.3);

	vec4 dist = startColor - endColor;

	outColor = vec4(0.909, 0.431, 0.156, 0.9) - (time / 60.0) * dist;
}
