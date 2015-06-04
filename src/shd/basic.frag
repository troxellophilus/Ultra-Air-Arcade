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

vec3 forestDiffuse = vec3(0.1, 0.35, 0.1);
vec3 forestAmbient = vec3(0.0, 0.0, 0.0) * forestDiffuse;

vec3 sandDiffuse = vec3(0.7, 0.53, 0.24);
vec3 sandAmbient = vec3(0.0, 0.0, 0.0) * sandDiffuse;

vec3 rocksDiffuse = vec3(0.507, 0.507, 0.507);
vec3 rocksAmbient = vec3(0.192, 0.192, 0.192) * rocksDiffuse;

vec3 snowDiffuse = vec3(0.5, 0.5, 0.5);
vec3 snowAmbient = vec3(0.05, 0.05, 0.05) * snowDiffuse;

vec3 woodDiffuse = vec3(0.3, 0.2, 0.05);
vec3 woodAmbient = vec3(0.6, 0.41, 0.29);// * woodDiffuse;

vec3 waterDiffuse = vec3(0.4, 0.5, 0.7);
vec3 waterAmbient = vec3(0.0, 0.05, 0.07) * waterDiffuse;

vec3 toonShade() {
    vec3 n = normalize(silh_vNor);
    vec3 e = normalize( vec3( 0.0, 0.0, 0.0 ) - silh_vPos );

    if (dot(n, e) < 0.15)
        return vec3(0.0, 0.0, 0.0);

    vec3 lightVector = normalize(lPos.xyz - vPos);
    vec3 ambient;
    vec3 diffuse;
    float cosine = dot(lightVector, vNor);
    float dotProduct = dot(normalize(vNor), vec3(0, 1, 0));

    if (vPos.y < 40) {
    	if (dotProduct > 0.6) {
			if (vPos.y < 0.5) {
				ambient = waterAmbient;
				diffuse = waterDiffuse;
			} else {
				ambient = forestAmbient;
				diffuse = forestDiffuse;
			}
    	} else if (dotProduct > 0.35) {
    		ambient = sandAmbient;
		diffuse = sandDiffuse;
    	} else {
    		ambient = woodAmbient;
		diffuse = woodDiffuse;
    	}
    } else {
    	if (dotProduct > 0.6) {
    		ambient = snowAmbient;
		diffuse = snowDiffuse;
    	} else if (dotProduct > 0.25) {
    		ambient = rocksAmbient;
		diffuse = rocksDiffuse;
    	} else {
    		ambient = woodAmbient;
		diffuse = woodDiffuse;
    	}
    }
    
    diffuse *= floor( cosine * levels ) * scaleFactor;
    
	if (renderObj == 1) { // Rendering for airplanes
		ambient = UaColor;
		float cosine = dot(lightVector, vNor);
		diffuse = UdColor * floor( cosine * levels ) * scaleFactor; 
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
	if (renderObj == 0 || renderObj == 1) {			// For rendering Characters and Terrain
		float dist = length(lPos.xyz - vPos);
		float intensity = (500 * dist) / (dist * dist + dist + 1);

		outColor = vec4(intensity * toonShade(), 1.0);
	} else if (renderObj == 2) {	// For rendering billboards
			outColor = vec4(1.0, 1.0, 1.0, 0.5);
	} else if (renderObj == 3) {
		vec3 n = normalize(silh_vNor);
    		vec3 e = normalize( vec3( 0.0, 0.0, 0.0 ) - silh_vPos );

    		if (dot(n, e) < 0.2)
        		outColor = vec4(1.0, 0.0, 1.0, 1.0);
        	else
        		outColor = vec4(0.0,0.0,0.0,0.0);
	} 
	else if(renderObj == 4)
	{
		vec3 n = normalize(silh_vNor);
    		vec3 e = normalize( vec3( 0.0, 0.0, 0.0 ) - silh_vPos );

    		if (dot(n, e) < 0.2)
        		outColor = vec4(0.0, 0.0, 0.0, 1.0);
        	else
        		outColor = vec4(forestDiffuse,1.0);
	}
	else {								// Catch-all
			outColor = vec4(vCol, 1.0);
	}
}
