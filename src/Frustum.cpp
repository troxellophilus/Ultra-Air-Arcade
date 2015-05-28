#include "Frustum.h"

Frustum::Frustum() {

}

Frustum::~Frustum() {

}

void Frustum::normalize(int side) {
	float magnitude = sqrt(pow(viewFrustum[side].getA(), 2) +
						   pow(viewFrustum[side].getB(), 2) +
						   pow(viewFrustum[side].getC(), 2));

	viewFrustum[side].setA(viewFrustum[side].getA() / magnitude);
	viewFrustum[side].setB(viewFrustum[side].getB() / magnitude);
	viewFrustum[side].setC(viewFrustum[side].getC() / magnitude);
	viewFrustum[side].setD(viewFrustum[side].getD() / magnitude);
}

void Frustum::setFrustum(glm::mat4 viewMatrix, glm::mat4 projMatrix) {
	glm::mat4 clip = projMatrix * viewMatrix;

//LEFT PLANE
	viewFrustum[LEFTP].setA(clip[0][3] + clip[0][0]);
	viewFrustum[LEFTP].setB(clip[1][3] + clip[1][0]);
	viewFrustum[LEFTP].setC(clip[2][3] + clip[2][0]);
	viewFrustum[LEFTP].setD(clip[3][3] + clip[3][0]);
	normalize(LEFTP);

//RIGHT PLANE
	viewFrustum[RIGHTP].setA(clip[0][3] - clip[0][0]);
	viewFrustum[RIGHTP].setB(clip[1][3] - clip[1][0]);
	viewFrustum[RIGHTP].setC(clip[2][3] - clip[2][0]);
	viewFrustum[RIGHTP].setD(clip[3][3] - clip[3][0]);
	normalize(RIGHTP);

//BOTTOM PLANE
	viewFrustum[BOTTOMP].setA(clip[0][3] + clip[0][1]);
	viewFrustum[BOTTOMP].setB(clip[1][3] + clip[1][1]);
	viewFrustum[BOTTOMP].setC(clip[2][3] + clip[2][1]);
	viewFrustum[BOTTOMP].setD(clip[3][3] + clip[3][1]);
	normalize(BOTTOMP);

//TOP PLANE
	viewFrustum[TOPP].setA(clip[0][3] - clip[0][1]);
	viewFrustum[TOPP].setB(clip[1][3] - clip[1][1]);
	viewFrustum[TOPP].setC(clip[2][3] - clip[2][1]);
	viewFrustum[TOPP].setD(clip[3][3] - clip[3][1]);
	normalize(TOPP);

//NEAR PLANE
	viewFrustum[NEARP].setA(clip[0][3] + clip[0][2]);
	viewFrustum[NEARP].setB(clip[1][3] + clip[1][2]);
	viewFrustum[NEARP].setC(clip[2][3] + clip[2][2]);
	viewFrustum[NEARP].setD(clip[3][3] + clip[3][2]);
	normalize(NEARP);

//FAR PLANE
	viewFrustum[FARP].setA(clip[0][3] - clip[0][2]);
	viewFrustum[FARP].setB(clip[1][3] - clip[1][2]);
	viewFrustum[FARP].setC(clip[2][3] - clip[2][2]);
	viewFrustum[FARP].setD(clip[3][3] - clip[3][2]);
	normalize(FARP);
}

bool Frustum::sphereInFrustum(glm::vec3 pos, float r) {
	for (int i = 0; i < 6; i++) {
		float value = viewFrustum[i].getA() * pos.x + viewFrustum[i].getB() * pos.y + viewFrustum[i].getC() * pos.z + viewFrustum[i].getD();
		// std::cout << i << ": " << value << std::endl;
		if (value <= -r) {
			return false;
		}
	}

	return true;
}