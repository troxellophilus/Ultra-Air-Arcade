attribute vec4 vertPos; // in object space
attribute vec3 vertNor; // in object space
uniform mat4 P;
uniform mat4 MV;
varying vec3 color;

void main()
{
	//float diffuseLight = 0;
   gl_Position = P * MV * vertPos;
	vec3 l = vec3(0.0, 0.0, 1.0); // In camera space
	vec4 nor = vec4(vertNor, 0);
   nor = MV * nor;

   color = vec3(.0,.4,.0) * max(0, dot(l, nor.xyz));

   //color = 0.5 * (nor + 1.0);
}
