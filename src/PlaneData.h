#ifndef PlANEDATA_H
#define PlANEDATA_H

enum Plane {
	//the six planes that make up the frustum
	LEFTP = 0, RIGHTP = 1, BOTTOMP = 2, TOPP = 3, NEARP = 4, FARP = 5
};

//Coefficients of the plane equation
class PlaneData {
private:
	float A;				// The X value of the plane's normal
	float B;				// The Y value of the plane's normal
	float C;				// The Z value of the plane's normal
	float D;				// The distance the plane is from the origin

public:
	float getA() { return A; }
	float getB() { return B; }
	float getC() { return C; }
	float getD() { return D; }
	void setA(float a) { A = a; }
	void setB(float b) { B = b; }
	void setC(float c) { C = c; }
	void setD(float d) { D = d; }
};

#endif