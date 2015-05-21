#version 330

uniform vec3 lPos;
uniform int renderObj;

uniform vec3 UaColor;
uniform vec3 UsColor;
uniform vec3 UdColor;
uniform float Ushine;

in vec3 silh_vPos;
in vec3 silh_vNor;

in vec3 vCol;
in vec3 vNor;
in vec3 vPos;

out vec4 outColor;

const int levels = 5;
const float scaleFactor = 1.0 / levels;

vec4 calcColor(vec3 fadeTo, vec3 fadeFrom, float dist, float bottom) {
	float red = fadeTo.r - (fadeTo.r - fadeFrom.r) * (vPos.y - bottom) / (dist);
	float green = fadeTo.g - (fadeTo.g - fadeFrom.g) * (vPos.y - bottom) / (dist);
	float blue = fadeTo.b - (fadeTo.b - fadeFrom.b) * (vPos.y - bottom) / (dist);

	return vec4(red, green, blue, 1.0);
}

vec3 toonShade()
{
    vec3 n = normalize(silh_vNor);
    vec3 e = normalize( vec3(-silh_vPos));

    vec3 lightVector = normalize(lPos - vPos);
    vec3 ambient = UaColor;
    float cosine = dot(lightVector, vNor);
    vec3 diffuse = UdColor * floor( cosine * levels ) * scaleFactor;

    if (dot(n, e) < 0.15)
    {
    	ambient = vec3(0.0f,0.0f,0.0f);
    	diffuse = vec3(0.0f,0.0f,0.0f);
    }

    return ambient + diffuse;
}

void main() {
	// FOR PHONG SHADING
	if (renderObj == 0) {			// For rendering Characters and Terrain
		
		float dist = length(lPos - vPos);
		float intensity = (500 * dist) / (dist * dist + dist + 1);

		outColor = vec4(intensity * toonShade(), 1.0);
	} 
	else if (renderObj == 1) { 	// For rendering skydome
		if (vPos.y < 50.0)
			outColor = calcColor(vec3(0.03, 0.874, 1.0), vec3(0.0745, 0.815, 1.0), 50.0, 0.0);
		else if (vPos.y >= 50.0 && vPos.y <= 250.0)
			outColor = calcColor(vec3(0.0745, 0.815, 1.0), vec3(0.058, 0.725, 1.0), 200.0, 50.0);
		else
			outColor = calcColor(vec3(0.058, 0.725, 1.0), vec3(0.007, 0.407, 1.0), 250.0, 250.0);
	} 
	else if (renderObj == 2) {	// For rendering billboards
			outColor = vec4(1.0, 1.0, 1.0, 0.5);
	} 
	else {								// Catch-all
			outColor = vec4(vCol, 1.0);
	}
}
