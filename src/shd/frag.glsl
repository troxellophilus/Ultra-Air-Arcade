#version 330

uniform vec3 lPos;
uniform int renderObj;

uniform vec3 UaColor;
uniform vec3 UsColor;
uniform vec3 UdColor;
uniform float Ushine;

in vec3 vCol;
in vec3 vNor;
in vec3 vPos;

out vec4 outColor;

vec4 calcColor(vec3 fadeTo, vec3 fadeFrom, float dist, float bottom) {
	float red = fadeTo.r - (fadeTo.r - fadeFrom.r) * (vPos.y - bottom) / (dist);
	float green = fadeTo.g - (fadeTo.g - fadeFrom.g) * (vPos.y - bottom) / (dist);
	float blue = fadeTo.b - (fadeTo.b - fadeFrom.b) * (vPos.y - bottom) / (dist);

	return vec4(red, green, blue, 1.0);
}

void main() {
	// FOR PHONG SHADING
	if (renderObj == 0) {			// For rendering Characters and Terrain
		vec3 diffuse_comp, specular_comp, ambient_comp;
		vec3 I_c = vec3(1.0, 1.0, 1.0);
		float dist = length(lPos - vPos);
		vec3 lightVector = normalize(lPos - vPos);

		vec3 V = vPos;
		vec3 L = normalize(lPos);
		vec3 H = normalize(V + L);
		vec3 R = -1.0 * L + 2.0 * max(dot(L, vNor), 0.0) * vNor; 

		diffuse_comp = UdColor * I_c * max(dot(vNor, L), 0.0);
		specular_comp = UsColor * I_c * pow(max(dot(vNor, R), 0.0), Ushine);
		ambient_comp = UaColor * I_c;

		float intensity = (500 * dist) / (dist * dist + dist + 1);

		outColor = vec4(intensity * (specular_comp + diffuse_comp + ambient_comp), 1.0);
	} else if (renderObj == 1) { 	// For rendering skydome
		if (vPos.y < 50.0)
			outColor = calcColor(vec3(0.03, 0.874, 1.0), vec3(0.0745, 0.815, 1.0), 50.0, 0.0);
		else if (vPos.y >= 50.0 && vPos.y <= 250.0)
			outColor = calcColor(vec3(0.0745, 0.815, 1.0), vec3(0.058, 0.725, 1.0), 200.0, 50.0);
		else
			outColor = calcColor(vec3(0.058, 0.725, 1.0), vec3(0.007, 0.407, 1.0), 250.0, 250.0);
	} else if (renderObj == 2) {	// For rendering billboards
			outColor = vec4(1.0, 1.0, 1.0, 0.5);
	} else {								// Catch-all
			outColor = vec4(vCol, 1.0);
	}
}
