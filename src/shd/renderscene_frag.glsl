#version 330 core

in vec3 vPos;
in vec3 vNor;

in vec3 eye;
in vec3 lightDirection;
in vec4 shadowCoord;
in vec3 silh_vPos;
in vec3 silh_vNor;
in vec3 lightPosition;

// Values that stay constant for the whole mesh.
uniform sampler2DShadow shadowMap;

out vec4 outColor;

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

const int levels = 20;
const float scaleFactor = 1.0 / levels;

vec2 poissonFilter[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

void main(){

	// Light emission properties
	vec3 lightColor = vec3(1,1,1);
	float shininess = 1.0f;
	
	// Material properties
	vec3 n = normalize( vNor );
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize( lightDirection );
	float Kt = clamp( dot( n,l ), 0, 1 );
	float Ka = clamp( dot( normalize(eye), reflect(-l, n) ), 0,1 );
	
	float lightFactor = 1.0;

	float bias = 0.005;
	// Sample the shadow map 4 times
	for (int i=0;i<16;i++)
		lightFactor -= 0.2*(1.0-texture( shadowMap, vec3(shadowCoord.xy + poissonFilter[i]/700.0,  (shadowCoord.z-bias)/shadowCoord.w) ));


	vec3 ambient = vec3(0.0);
	vec3 diffuse = vec3(0.0);
	vec3 lightVector = normalize(lightPosition - vPos);
	float cosine = dot(lightVector, vNor);
	float dotProduct = dot(normalize(vNor), vec3(0, 1, 0));
	float dist = length(lightPosition - vPos);
	float intensity = (500 * dist) / (dist * dist + dist + 1);
    
    if (vPos.y < 40) {
    	if (dotProduct > 0.6) {
    		if (vPos.y < 0.5) {
				ambient = waterAmbient;
				diffuse = waterDiffuse;
			} else {
				ambient = forestAmbient;
				diffuse = forestDiffuse;
			}
    	} else if (dotProduct > 0.25) {
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
    
    if (dot(normalize(silh_vNor), normalize( vec3( 0.0, 0.0, 0.0 ) - silh_vPos )) < 0.15) {
    	ambient = vec3(0.0, 0.0, 0.0);
    	diffuse = vec3(0.0, 0.0, 0.0);
    }
    
    diffuse *= lightFactor * lightColor * shininess * Kt * floor( cosine * levels ) * scaleFactor;
    
	outColor = vec4((ambient + diffuse) * intensity, 1.0);
}
