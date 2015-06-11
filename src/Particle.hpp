#ifndef PARTICLE_HPP
#define PARTICLE_HPP
#define NUM_PARTICLES 60

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "helper.h"
#include "GLSL.h"
#include <SOIL/SOIL.h>
#include <png.h>

class Particle {
	private:
		int particleIndex;
		int frameCount;
		// Items necessary for shaders
		GLuint prog;
		GLuint vbo;

		GLuint CameraRight_worldspace_ID;
		GLuint CameraUp_worldspace_ID;
		GLuint BillboardPosID;
		GLuint BillboardSizeID;
		GLuint TimeStepID;
		GLuint posAttrib;

		GLuint uProjMatrix;
		GLuint uViewMatrix;
		// End list
		float randFactor[NUM_PARTICLES];
		bool toRender[NUM_PARTICLES];
		int timeStep[NUM_PARTICLES];

		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		glm::mat4 rotMatrix;
		glm::vec3 position;
		glm::quat quaternion;

		float randNum();
	public:
		Particle();
		void update(glm::mat4, glm::mat4, glm::quat, glm::vec3);
		void draw(float);
		void setShaderProg(GLuint);
};

Particle::Particle() {
	int i;
	for (i = 0; i < NUM_PARTICLES; i++) {
		timeStep[i] = 0;
		toRender[i] = false;
		randFactor[i] = randNum();
	} 

	particleIndex = 0;
	frameCount = 0;
}

void Particle::update(glm::mat4 viewMat, glm::mat4 projMat, glm::quat rot, glm::vec3 pos) {
	viewMatrix = viewMat;
	projMatrix = projMat;
	position = pos;
	quaternion = rot;

	if (frameCount < NUM_PARTICLES) toRender[frameCount++] = true;
}

void Particle::draw(float thrust) {
	glUseProgram(prog);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform3f(CameraRight_worldspace_ID, viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
	glUniform3f(CameraUp_worldspace_ID   , viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);

	// Set projection matrix
    glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, glm::value_ptr(projMatrix));

    // Set view matrix
    glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform2f(BillboardSizeID, 0.025f, 0.025f);

	// Send position array to GPU
	glEnableVertexAttribArray(posAttrib);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	int i;
	for (i = 0; i < NUM_PARTICLES; i++) {
		if (toRender[i]) {
			float factor = (-thrust) * randFactor[i] * timeStep[i] * 0.07 / NUM_PARTICLES;
			glm::vec3 direction = (0.15f + factor) * glm::normalize(glm::vec3(0, -0.25, 1) * glm::inverse(quaternion));
			glUniform3f(BillboardPosID, position.x + direction.x, position.y + direction.y, position.z + direction.z);
			glUniform2f(BillboardSizeID, 0.025f, 0.025f);
			glUniform1i(TimeStepID, timeStep[i]);

			if (timeStep[i] == NUM_PARTICLES -1) timeStep[i] = 0;
			else timeStep[i]++;

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

	glDisableVertexAttribArray(posAttrib);

	glUseProgram(0);
	GLSL::printError(__FILE__, __LINE__);
	assert(glGetError() == GL_NO_ERROR);
}

float Particle::randNum() {
    return ((float) rand() / (RAND_MAX)) * 1.005;
}

void Particle::setShaderProg(GLuint programID) {
	prog = programID;
    glUseProgram(prog);
    
    // Set up the shader variables
    posAttrib = glGetAttribLocation(prog, "position");
    uViewMatrix = glGetUniformLocation(prog, "V");
    uProjMatrix = glGetUniformLocation(prog, "P");

	// Shader variables for billboard
	CameraRight_worldspace_ID  = glGetUniformLocation(prog, "CameraRight_worldspace");
	CameraUp_worldspace_ID  = glGetUniformLocation(prog, "CameraUp_worldspace");
	BillboardPosID = glGetUniformLocation(prog, "particlePos");
	BillboardSizeID = glGetUniformLocation(prog, "particleSize");
	TimeStepID = glGetUniformLocation(prog, "timeStep");
	
	static constexpr GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f
	};
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
}

#endif
