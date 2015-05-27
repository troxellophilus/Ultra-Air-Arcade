#version 330 core

in vec3 position_worldspace;
in vec3 normal_cameraspace;
in vec3 eye;
in vec3 lightDirection_cameraspace;
in vec4 shadowCoord;

out vec3 outColor;

// Values that stay constant for the whole mesh.
uniform sampler2DShadow shadowMap;

vec3 MaterialDiffuseColor = vec3(0.3, 0.2, 0.05);
vec3 MaterialAmbientColor = vec3(0.6,0.41,0.29) * MaterialDiffuseColor;
vec3 MaterialSpecularColor = vec3(0.0,0.0,0.0);

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
	vec3 n = normalize( normal_cameraspace );
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize( lightDirection_cameraspace );
	float Kt = clamp( dot( n,l ), 0, 1 );
	float Ka = clamp( dot( normalize(eye), reflect(-l, n) ), 0,1 );
	
	float lightFactor = 1.0;

	float bias = 0.005;
	// Sample the shadow map 4 times
	for (int i=0;i<16;i++)
		lightFactor -= 0.2*(1.0-texture( shadowMap, vec3(shadowCoord.xy + poissonFilter[i]/700.0,  (shadowCoord.z-bias)/shadowCoord.w) ));


	vec3 ambient = MaterialAmbientColor;
	vec3 diffuse = lightFactor * MaterialDiffuseColor * lightColor * shininess * Kt;
	vec3 specular = lightFactor * MaterialSpecularColor * lightColor * shininess * pow(Ka,5);

	outColor = ambient + specular + diffuse;
}
