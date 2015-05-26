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

vec3 waterAmbient = vec3(0.0, 0.05, 0.07);
vec3 waterDiffuse = vec3(0.4, 0.5, 0.7);
vec3 waterSpecular = vec3(0.04, 0.7, 0.7);

vec3 forestAmbient = vec3(0.0, 0.0, 0.0);
vec3 forestDiffuse = vec3(0.1, 0.35, 0.1);

vec3 sandAmbient = vec3(0.0, 0.0, 0.0);
vec3 sandDiffuse = vec3(0.5, 0.5, 0.0);

vec3 rocksAmbient = vec3(0.192, 0.192, 0.192);
vec3 rocksDiffuse = vec3(0.507, 0.507, 0.507);

vec3 snowAmbient = vec3(0.05, 0.05, 0.05);
vec3 snowDiffuse = vec3(0.5, 0.5, 0.5);
 
vec3 toonShade()
{
    vec3 n = normalize(silh_vNor);
    vec3 e = normalize( vec3(-silh_vPos));

	vec3 ambient, diffuse;
    if (dot(n, e) < 0.15)
        return vec3(0.0, 0.0, 0.0);

    vec3 lightVector = normalize(lPos - vPos);
	float dotProduct = dot(vNor, vec3(0, 1, 0));

	if (dotProduct > 2 && vPos.y > 50) { //&& vPos.y > 30) { //&& vPos.y >= 90) {
		ambient = snowAmbient;
		float cosine = dot(lightVector, vNor);
		diffuse = snowDiffuse * floor( cosine * levels ) * scaleFactor;
    } else if (dotProduct > 1.5 && vPos.y > 10) {
		ambient = UaColor;
		float cosine = dot(lightVector, vNor);
		diffuse = UdColor * floor( cosine * levels ) * scaleFactor;
	} else if (dotProduct > 1.0 && vPos.y < 50) {
		ambient = forestAmbient;
		float cosine = dot(lightVector, vNor);
		diffuse = forestDiffuse * floor( cosine * levels ) * scaleFactor;
	} else if (dotProduct > 0.75) {
		ambient = rocksAmbient;
		float cosine = dot(lightVector, vNor);
		diffuse = rocksDiffuse * floor( cosine * levels ) * scaleFactor;
	} else if (dotProduct > 0.25) {
		ambient = sandAmbient;
		float cosine = dot(lightVector, vNor);
		diffuse = sandDiffuse * floor( cosine * levels ) * scaleFactor;
	} else if (dotProduct > 0.05){
		ambient = forestAmbient;
		float cosine = dot(lightVector, vNor);
		diffuse = forestDiffuse * floor( cosine * levels ) * scaleFactor;
	} else if (vPos.y < 0.2) {
		ambient = waterAmbient;
		float cosine = dot(lightVector, vNor);
		diffuse = waterDiffuse * floor( cosine * levels ) * scaleFactor;
	}

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
	} else if (renderObj == 2) {	// For rendering billboards
			outColor = vec4(1.0, 1.0, 1.0, 0.5);
	} else {								// Catch-all
			outColor = vec4(vCol, 1.0);
	}
}
