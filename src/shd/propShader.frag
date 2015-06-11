#version 330

uniform vec3 lPos;
uniform sampler2D tex_un;

in vec3 vCol;
in vec3 vNor;
in vec3 vPos;
in vec2 fragTex;

out vec4 outColor;

void main()
{
	float dist = length(lPos - vPos);
	vec3 light_direction = normalize(lPos - vPos);
	float intensity = (1000 * dist) / (dist * dist + dist + 1);

	vec3 tex = texture(tex_un, fragTex.st).rgb;
	outColor = vec4(vCol * max(dot(vNor,light_direction) , 0.0), 1.0f);
}

