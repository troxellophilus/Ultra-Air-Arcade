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

void main() {
   // FOR PHONG SHADING
   if (renderObj != 0) {
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
      	outColor = vec4(specular_comp + diffuse_comp + ambient_comp, 1.0);
} else {
      outColor = vec4(vCol, 1.0);
   }
}
