attribute vec4 aPos;
attribute vec3 aNor;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform vec3 lPos;
uniform int renderObj;
uniform vec3 UaColor;
uniform vec3 UsColor;
uniform vec3 UdColor;
uniform float Ushine;

varying vec3 vNor;
varying vec3 vPos;
varying vec3 vCol;

void main() {
   // FOR PHONG SHADING 
   if (renderObj != 0) {
      vec3 diffuse_comp, specular_comp, ambient_comp;
      vec3 I_c = vec3(1.0, 1.0, 1.0);
      vPos = vec3(M * aPos);
      vCol = I_c;
      vNor = vec3(M * vec4(aNor, 0.0));
      gl_Position = P * V * M * aPos;
   } else {
      gl_Position = P * V * M * aPos;
      vCol = vec3(0.1, 0.5, 0.3);
   }
}
