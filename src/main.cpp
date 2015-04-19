
#include <GLFW/glfw3.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "ShapeObj.h"

using namespace std;

bool keyToggles[256] = {false};

float t = 0.0f;
float tPrev = 0.0f;
int width = 1;
int height = 1;

Program prog;
Camera camera;
ShapeObj bunny;

void loadScene()
{
	t = 0.0f;
	keyToggles['c'] = true;
	
	bunny.load("bunny.obj");
	prog.setShaderNames("simple_vert.glsl", "simple_frag.glsl");
}

void initGL()
{
	//////////////////////////////////////////////////////
	// Initialize GL for the whole scene
	//////////////////////////////////////////////////////
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	//////////////////////////////////////////////////////
	// Intialize the shapes
	//////////////////////////////////////////////////////
	
	bunny.init();
	
	//////////////////////////////////////////////////////
	// Intialize the shaders
	//////////////////////////////////////////////////////
	
	prog.init();
	prog.addUniform("P");
	prog.addUniform("MV");
	prog.addAttribute("vertPos");
	prog.addAttribute("vertNor");
	
	//////////////////////////////////////////////////////
	// Final check for errors
	//////////////////////////////////////////////////////
	GLSL::checkVersion();
}

/*still trying to understand the difference between setWindowSize and setFrameBufferSize*/
void reshapeBufferGL(GLFWwindow* window, int w, int h)
{
	// Set view size
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	//camera.setAspect((float)width/(float)height);
}
/*NOTE: there is also a closed callback if needed*/
void reshapeGL(GLFWwindow* window, int w, int h)
{
	// Set view size
	width = w;
	height = h;
	glfwSetWindowSize(window, width, height);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	camera.setAspect((float)width/(float)height);
}

void drawGL()
{
	// Elapsed time
	float tCurr = (float)glfwGetTime();
	if(keyToggles[' ']) {
		t += (tCurr - tPrev);
	}
	tPrev = tCurr;
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles['c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles['l']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	//////////////////////////////////////////////////////
	// Create matrix stacks
	//////////////////////////////////////////////////////
	
	MatrixStack P, MV;
	P =  MatrixStack();
	MV = MatrixStack();
	// Apply camera transforms
	P.pushMatrix();
	camera.applyProjectionMatrix(&P);
	MV.pushMatrix();
	camera.applyViewMatrix(&MV);
	
	//////////////////////////////////////////////////////
	// Draw origin frame using old-style OpenGL
	// (before binding the program)
	//////////////////////////////////////////////////////
	// Setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(P.topMatrix().data());
	
	// Setup the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(MV.topMatrix().data());
	
	// Draw frame
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glEnd();
	glLineWidth(1);
	
	// Pop modelview matrix
	glPopMatrix();
	
	// Pop projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Draw string
	P.pushMatrix();
	P.ortho(0, width, 0, height, -1, 1);
	glPushMatrix();
	glLoadMatrixf(P.topMatrix().data());
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRasterPos2f(5.0f, 5.0f);
	char str[256];
	sprintf(str, "Rotation: %s%s%s -> %s%s%s",
			(keyToggles['x'] ? "x" : "_"),
			(keyToggles['y'] ? "y" : "_"),
			(keyToggles['z'] ? "z" : "_"),
			(keyToggles['X'] ? "X" : "_"),
			(keyToggles['Y'] ? "Y" : "_"),
			(keyToggles['Z'] ? "Z" : "_"));
	for(int i = 0; i < strlen(str); ++i) {
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	P.popMatrix();
	
	//////////////////////////////////////////////////////
	// Now draw the shape using modern OpenGL
	//////////////////////////////////////////////////////
	
	// Bind the program
	prog.bind();
	
	// Send projection matrix (same for all bunnies)
	glUniformMatrix4fv(prog.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	
	// Apply some transformations to the modelview matrix.
	// Each bunny should get a different transformation.
	
	// The center of the bunny is at (-0.2802, 0.932, 0.0851)
	Eigen::Vector3f center(-0.2802, 0.932, 0.0851);
	//MV.translate(-center);
	
	// Alpha is the linear interpolation parameter between 0 and 1
	float alpha = std::fmod(t, 1.0f);
	
	// The axes of rotatio for the source and target bunnies
	Eigen::Vector3f axis0, axis1;
	axis0(0) = keyToggles['x'] ? 1.0 : 0.0f;
	axis0(1) = keyToggles['y'] ? 1.0 : 0.0f;
	axis0(2) = keyToggles['z'] ? 1.0 : 0.0f;
	axis1(0) = keyToggles['X'] ? 1.0 : 0.0f;
	axis1(1) = keyToggles['Y'] ? 1.0 : 0.0f;
	axis1(2) = keyToggles['Z'] ? 1.0 : 0.0f;
	if(axis0.norm() > 0.0f) {
		axis0.normalize();
	}
	if(axis1.norm() > 0.0f) {
		axis1.normalize();
	}
	
	Eigen::Quaternionf q0, q1;
	q0 = Eigen::AngleAxisf(90.0f/180.0f*M_PI, axis0);
	q1 = Eigen::AngleAxisf(90.0f/180.0f*M_PI, axis1);

	Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
	
	Eigen::Vector3f p0, p1, lerp;
	p0 << -1.0f, 1.0f, 1.0f;
	p1 <<  1.0f, -2.0f, 2.0f;
	lerp = alpha * p1 + (1-alpha) * p0;
	
	MV.pushMatrix();
		
		MV.pushMatrix();
			R.block<3,3>(0,0) = q0.toRotationMatrix();
			MV.translate(p0);
			MV.multMatrix(R);
			MV.translate(-center);		
			glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
			bunny.draw(prog.getAttribute("vertPos"), prog.getAttribute("vertNor"), -1);
		MV.popMatrix();

		MV.pushMatrix();
			R.block<3,3>(0,0) = q1.toRotationMatrix();
			MV.translate(p1);
			MV.multMatrix(R);
			MV.translate(-center);
			glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
			bunny.draw(prog.getAttribute("vertPos"), prog.getAttribute("vertNor"), -1);
		MV.popMatrix();

		MV.pushMatrix();
			R.block<3,3>(0,0) = q0.slerp(alpha,q1).toRotationMatrix();
			MV.translate(alpha*p1 + (1-alpha) * p0);
			MV.multMatrix(R);
			MV.translate(-center);
			glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
			bunny.draw(prog.getAttribute("vertPos"), prog.getAttribute("vertNor"), -1);
		MV.popMatrix();

	MV.popMatrix();
	
	// Unbind the program
	prog.unbind();

	//////////////////////////////////////////////////////
	// Cleanup
	//////////////////////////////////////////////////////
	
	// Pop stacks
	MV.popMatrix();
	P.popMatrix();	
}
/**
This is the function signature for mouse button callback functions.

Parameters
[in]	window	The window that received the event.
[in]	button	The mouse button that was pressed or released.
[in]	action	One of GLFW_PRESS or GLFW_RELEASE.
[in]	mods	Bit field describing which modifier keys were held down.

*/
void mouseGL(GLFWwindow* window, int button, int action, int mods)
{
	bool shift = mods & GLFW_MOD_SHIFT;
	bool ctrl  = mods & GLFW_MOD_CONTROL;
	bool alt   = mods & GLFW_MOD_ALT;

	camera.mouseClicked(shift, ctrl, alt);
}

void mouseMotionGL(GLFWwindow* window, double x, double y)
{
	camera.mouseMoved(x, y);
}
/*
funtion format for using glfwSetKeyCallback

	[in]	window	The window that received the event.
	[in]	key	The keyboard key that was pressed or released.
	[in]	scancode	The system-specific scancode of the key.
	[in]	action	GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
	[in]	mods	Bit field describing which modifier keys were held down.

*/
static void keyboardGL(GLFWwindow *window,int key, int scancode, int action, int mods)
{
	keyToggles[key] = !keyToggles[key];
	switch(key) {
		case 27:
			// ESCAPE glfw, then exit
			glfwTerminate();
			exit(0);
			break;
		case 't':
			t = 0.0f;
			break;
	}
}

void timerGL()
{
	//time is a double in SECONDS
	glfwGetTime();
}
void glfwMainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &width, &height);
        
        drawGL();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
/*aobut GLFWwindow:

[in]	width	The desired width, in screen coordinates, of the window. This must be greater than zero.
[in]	height	The desired height, in screen coordinates, of the window. This must be greater than zero.
[in]	title	The initial, UTF-8 encoded window title.
[in]	monitor	The monitor to use for full screen mode, or NULL to use windowed mode.
	use  glfwGetPrimaryMonitor() for full screen
[in]	share	The window whose context to share resources with, or NULL to not share resources.
*/
int main(int argc, char **argv)
{
	//initialize glfw. exit if failed
	if (!glfwInit())
    	exit(EXIT_FAILURE);

    //creates a window with width by height, then chekcs return
	GLFWwindow* window = glfwCreateWindow(1000, 1000, "Test Game", NULL, NULL);
	if (!window)
	{
    	glfwTerminate();
    	exit(EXIT_FAILURE);
	}
	//if windowed and want to move to another position
	//glfwSetWindowPos(window, 100, 100);
	
	//make openGL context for window
	glfwMakeContextCurrent(window);

	glfwSetCursorPosCallback(window, mouseMotionGL);
	glfwSetKeyCallback(window, keyboardGL);	
	glfwSetWindowSizeCallback(window, reshapeGL);
	glfwSetFramebufferSizeCallback(window, reshapeBufferGL);
	
	loadScene();
	initGL();
	
	//run until done
	glfwMainLoop(window);

	//destroy and exit
	glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);

	return 0;
}
