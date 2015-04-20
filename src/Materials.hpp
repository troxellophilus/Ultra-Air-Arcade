/*
 * Static Class for organizing and holding materials
 *
 * To use this, just include it and call:
 *   Materials::emerald
 *   Materials::jade
 *   Materials::wood
 *   Materials::obsidian
 *   Materials::greenPlastic
 */

#ifndef MATERIALS_H
#define MATERIALS_H

#include <glm/glm.hpp>

class Material {
private:
	glm::vec3 amb;
	glm::vec3 dif;
	glm::vec3 spc;
	float shn;

public:
	Material();
	Material(glm::vec3,glm::vec3,glm::vec3,float);
	glm::vec3 getAmbient();
	glm::vec3 getDiffuse();
	glm::vec3 getSpecular();
	float getShininess();
};

Material::Material() :
	amb(1,0,1),
	dif(1,0,1),
	spc(1,0,1),
	shn(300.) {
}

Material::Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess) {
	amb = glm::vec3(ambient);
	dif = glm::vec3(diffuse);
	spc = glm::vec3(specular);
	shn = shininess;
}

glm::vec3 Material::getAmbient() {
	return amb;
}

glm::vec3 Material::getDiffuse() {
	return dif;
}

glm::vec3 Material::getSpecular() {
	return spc;
}

float Material::getShininess() {
	return shn;
}

class Materials {
public:
	static Material emerald;
	static Material jade;
	static Material wood;
	static Material obsidian;
	static Material greenPlastic;
};

Material Materials::emerald(glm::vec3(0.0215, 0.2745, 0.0215),
			glm::vec3(0.07568, 0.61424, 0.7568),
			glm::vec3(0.633, 0.727811, 0.633),
			150.8);

Material Materials::jade = Material(glm::vec3(0.135, 0.2225, 0.1575),
			glm::vec3(0.54, 0.89, 0.63),
			glm::vec3(0.54, 0.89, 0.63),
			100.8);

Material Materials::wood = Material(glm::vec3(0.3, 0.2, 0.05),
			glm::vec3(0.6, 0.41, 0.29),
			glm::vec3(0.0, 0.0, 0.0),
			3.0);

Material Materials::obsidian = Material(glm::vec3(0.05375, 0.05, 0.06626),
			glm::vec3(0.18275, 0.17, 0.22525),
			glm::vec3(0,0,0),
			//glm::vec3(0.332741, 0.328634, 0.346435),
			38.4);

Material Materials::greenPlastic = Material(glm::vec3(0.15, 0.4, 0.15),
			glm::vec3(0.1, 0.35, 0.1),
			glm::vec3(0.45, 0.55, 0.45),
			100.0);

#endif
