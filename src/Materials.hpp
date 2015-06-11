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
	static Material stone;
	static Material obsidian;
	static Material greenPlastic;
	static Material red;
	static Material brass;
	static Material turquoise;
	static Material pink;
};

Material Materials::emerald(glm::vec3(0.0315, 0.3745, 0.0315),
			glm::vec3(0.07568, 0.61424, 0.7568),
			glm::vec3(0.833, 0.927811, 0.733),
			150.8);

Material Materials::jade = Material(glm::vec3(0.135, 0.2225, 0.1575),
			glm::vec3(0.54, 0.89, 0.63),
			glm::vec3(0.54, 0.89, 0.63),
			100.8);

Material Materials::wood = Material(glm::vec3(0.3, 0.2, 0.05),
			glm::vec3(0.6, 0.41, 0.29),
			glm::vec3(0.0, 0.0, 0.0),
			3.0);

Material Materials::stone = Material(glm::vec3(0.25, 0.25, 0.2),
				glm::vec3(0.4, 0.4, 0.35),
				glm::vec3(0,0,0),
				1000.0);

Material Materials::obsidian = Material(glm::vec3(0.05375, 0.05, 0.06626),
			glm::vec3(0.18275, 0.17, 0.22525),
			glm::vec3(0,0,0),
			//glm::vec3(0.332741, 0.328634, 0.346435),
			38.4);

Material Materials::greenPlastic = Material(glm::vec3(0.15, 0.4, 0.15),
			glm::vec3(0.1, 0.35, 0.1),
			glm::vec3(0.45, 0.55, 0.45),
			100.0);

Material Materials::red = Material(glm::vec3(0.6, 0.0, 0.0),
			glm::vec3(0.1, 0.35, 0.1),
			glm::vec3(0.45, 0.55, 0.45),
			100.0);

Material Materials::brass = Material(glm::vec3(0.329412, 0.22352, 0.027451),
				glm::vec3(0.880392, 0.668627, 0.313725),
				glm::vec3(0.992157, 0.941176, 0.907843),
				67.8974);

Material Materials::turquoise = Material(glm::vec3(0.1, 0.18725,0.1745),
				glm::vec3(0.596, 0.94151, 0.79102),
				glm::vec3(0.497254, 0.60829, 0.606678),
				42.8);

Material Materials::pink = Material(glm::vec3(0.575, 0.255, 0.466),
				glm::vec3(0.878, 0.4, 0.975),
				glm::vec3(0.9, 0.8, 0.9),
				100.0);

#endif
