#version 330

uniform vec3 lPos;

uniform float roughness;
uniform float fresnel;
uniform float geometric;

in vec3 vCol;
in vec3 vNor;
in vec3 vPos;

out vec4 outColor;

float Gaussian (float x, float deviation)
{
   return (1.0 / sqrt(2.0 * 3.141592 * deviation)) * exp(-((x * x) / (2.0 * deviation)));
}

void main() {
   vec3 ambientLight = vCol; 
   vec3 lColor = vec3(1.0, 1.0, 1.0);
   vec3 lDirection = normalize(lPos);

   float NdotL = max(dot(normalize(vNor), lDirection), 0.0);

   float specular = 0.0;
   if(NdotL > 0.0) {
      vec3 eyeDir = normalize(vPos - lPos);

      // Calculate intermediary values
      vec3 halfVector = normalize(lDirection + eyeDir);
      float NdotH = max(dot(normalize(vNor), halfVector), 0.0); 
      float VdotH = max(dot(eyeDir, halfVector), 0.0);
      float mSquared = roughness * roughness;

      // Geometric attenuation
      float NH2 = 2.0 * NdotH;
      float g1 = (NH2 * NdotL) / VdotH;
      float g2 = (NH2 * NdotL) / VdotH;
      float geoAtt = min(1.0, min(g1, g2));

      // Roughness (or: microfacet distribution function)
      // Beckmann distribution function
      float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
      float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
      float roughness = r1 * exp(r2);

      // Fresnel
      // Schlick approximation
      float schlick = pow(1.0 - VdotH, 5.0);
      schlick *= (1.0 - fresnel);
      schlick += fresnel;

      specular = (schlick * geoAtt * roughness) / (NdotL * NdotL * 3.14);
   }
   float dist = length(lPos.xyz - vPos);
   float intensity = (1000 * dist) / (dist * dist + dist + 1);
   
   vec3 finalColor = intensity * (lColor * NdotL * (geometric + specular * (1.0 - geometric)) + ambientLight);
   outColor =  vec4(finalColor,1.0);
}


