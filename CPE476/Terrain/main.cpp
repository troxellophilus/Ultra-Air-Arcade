/*
	This is still in GLUT but the terrain class is only math and should be cross compatable with anything. The line that i
	believe draws mostly everything is the glDrawElements(mode, (int)indBuf.size(), GL_UNSIGNED_INT, 0); line. So if that still
	works this should work no matter what. Let me know if you have any questions!

	Billy
*/

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include <iostream>
#include <vector>
#include "GLSL.h"
#include "Camera.h"
#include "stdio.h"
#include "MatrixStack.h"
#include "Terrain.h"

using namespace std;

Camera camera;
vector<float> posBuf;
vector<float> norBuf;
vector<unsigned int> indBuf;
GLuint posBufID;
GLuint norBufID;
GLuint indBufID;
GLuint pid;
GLint h_vertPos;
GLint h_vertNor;
GLint h_P;
GLint h_MV;
bool cull = true;
int mode = GL_TRIANGLES;

float mongeZ(float x, float y)
{
	return 0.1 * (cos(10 * x) + sin(10 * y));

}
void toggleMode()
{
	mode = (mode == GL_TRIANGLES)?GL_LINES:GL_TRIANGLES;
}
void loadScene(int bumpSize)
{
   Terrain terrain = Terrain("Tamriel.bmp", 100.0, posBuf, indBuf, norBuf);
}

void initGL()
{
	//////////////////////////////////////////////////////
	// Initialize GL for the whole scene
	//
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	//////////////////////////////////////////////////////
	// Initialize the vertex buffers
	//
	
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array (if it exists) to the GPU
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	
	// Send the index array to the GPU
	glGenBuffers(1, &indBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	//////////////////////////////////////////////////////
	// Intialize the shaders
	//
	
	// Create shader handles
	string vShaderName = "lab6_vert.glsl";
	string fShaderName = "lab6_frag.glsl";
	GLint rc;
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
	
	// Read shader sources
	const char *vshader = GLSL::textFileRead(vShaderName.c_str());
	const char *fshader = GLSL::textFileRead(fShaderName.c_str());
	glShaderSource(VS, 1, &vshader, NULL);
	glShaderSource(FS, 1, &fshader, NULL);
	
	// Compile vertex shader
	glCompileShader(VS);
	GLSL::printError();
	glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(VS);
	if(!rc) {
		printf("Error compiling vertex shader %s\n", vShaderName.c_str());
	}
	
	// Compile fragment shader
	glCompileShader(FS);
	GLSL::printError();
	glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(FS);
	if(!rc) {
		printf("Error compiling fragment shader %s\n", fShaderName.c_str());
	}
	
	// Create the program and link
	pid = glCreateProgram();
	glAttachShader(pid, VS);
	glAttachShader(pid, FS);
	glLinkProgram(pid);
	GLSL::printError();
	glGetProgramiv(pid, GL_LINK_STATUS, &rc);
	GLSL::printProgramInfoLog(pid);
	if(!rc) {
		printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
	}
	
	h_vertPos = GLSL::getAttribLocation(pid, "vertPos");
	h_vertNor = GLSL::getAttribLocation(pid, "vertNor");
	h_P = GLSL::getUniformLocation(pid, "P");
	h_MV = GLSL::getUniformLocation(pid, "MV");

	// Check GLSL
	GLSL::checkVersion();
}

void reshapeGL(int w, int h)
{
	// Set view size
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	// Set camera aspect ratio
	camera.setAspect((float)w/h);
}

void drawGL()
{
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Enable backface culling
	if(cull) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	
	// Create matrix stacks
	MatrixStack P, MV;
	// Apply camera transforms
	P.pushMatrix();
	camera.applyProjectionMatrix(&P);
	MV.pushMatrix();
	camera.applyCameraMatrix(&MV);
	
	// Bind the program
	glUseProgram(pid);
	glUniformMatrix4fv( h_P, 1, GL_FALSE, glm::value_ptr( P.topMatrix()));
	glUniformMatrix4fv(h_MV, 1, GL_FALSE, glm::value_ptr(MV.topMatrix()));
	
	// Enable and bind position array
	GLSL::enableVertexAttribArray(h_vertPos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_vertPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Enable and bind normal array
	GLSL::enableVertexAttribArray(h_vertNor);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glVertexAttribPointer(h_vertNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Bind index array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
	
	// Draw
	glDrawElements(mode, (int)indBuf.size(), GL_UNSIGNED_INT, 0);
	
	// Disable and unbind
	GLSL::disableVertexAttribArray(h_vertNor);
	GLSL::disableVertexAttribArray(h_vertPos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	// Unbind the program
	glUseProgram(0);

	// Pop stacks
	MV.popMatrix();
	P.popMatrix();

	// Double buffer
	glutSwapBuffers();
}

void mouseGL(int button, int state, int x, int y)
{
	int modifier = glutGetModifiers();
	bool shift = modifier & GLUT_ACTIVE_SHIFT;
	bool ctrl  = modifier & GLUT_ACTIVE_CTRL;
	bool alt   = modifier & GLUT_ACTIVE_ALT;
	camera.mouseClicked(x, y, shift, ctrl, alt);
}

void motionGL(int x, int y)
{
	camera.mouseMoved(x, y);
	// Refresh screen
	glutPostRedisplay();
}

void keyboardGL(unsigned char key, int x, int y)
{
	//cout << key << endl;
	switch(key) {
		case 27:
			// ESCAPE
			exit(0);
			break;
		/*case 'c':
			cull = !cull;
			break;*/
		case 'l':
			toggleMode();
			break;
	}
	// Refresh screen
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	int bumpSize = 10;
	if(argc > 1)
		bumpSize = atoi(argv[1]);
	glutInit(&argc, argv);
	glutInitWindowSize(400, 400);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Your Name");
	glutMouseFunc(mouseGL);
	glutMotionFunc(motionGL);
	glutKeyboardFunc(keyboardGL);
	glutReshapeFunc(reshapeGL);
	glutDisplayFunc(drawGL);
	loadScene(bumpSize);
	initGL();
	glutMainLoop();
	return 0;
}
