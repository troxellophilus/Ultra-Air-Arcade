#ifndef PARTICLE_HPP
#define PARTICLE_HPP
#define NUM_PARTICLES 100

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "helper.h"
#include "GLSL.h"
#include <SOIL/SOIL.h>
#include <png.h>

class Particle {
	private:
		// Items necessary for shaders
		GLuint prog;
		GLuint vbo;

		GLuint CameraRight_worldspace_ID;
		GLuint CameraUp_worldspace_ID;
		GLuint BillboardPosID;
		GLuint BillboardSizeID;
		GLuint posAttrib;

		GLuint uProjMatrix;
		GLuint uViewMatrix;
		GLuint uModelMatrix;
		// End list
		float xtrans[NUM_PARTICLES];
		float ytrans[NUM_PARTICLES];
		float ztrans[NUM_PARTICLES];

		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		glm::mat4 rotMatrix;
		glm::vec3 position;

		float randNum();
	public:
		Particle();
		void update(glm::mat4, glm::mat4, glm::mat4, glm::vec3);
		void draw();
		void setShaderProg(GLuint);
};

Particle::Particle() {
	int i;
	for (i = 0; i < NUM_PARTICLES; i++) {
		xtrans[i] = randNum();
		ytrans[i] = randNum();
		ztrans[i] = randNum();
	} 

    assert(glGetError() == GL_NO_ERROR);
}

void Particle::update(glm::mat4 viewMat, glm::mat4 projMat, glm::mat4 rotMat, glm::vec3 pos) {
	viewMatrix = viewMat;
	projMatrix = projMat;
	rotMatrix = rotMat;
	position = pos;
}

void Particle::draw() {
	glUseProgram(prog);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform3f(CameraRight_worldspace_ID, viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
	glUniform3f(CameraUp_worldspace_ID   , viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);

	// Set projection matrix
    glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, glm::value_ptr(projMatrix));

    // Set view matrix
    glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glm::mat4 model = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, 0.2));
    glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(model));

	// Send position array to GPU
	glEnableVertexAttribArray(posAttrib);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	int i;
	for (i = 0; i < NUM_PARTICLES; i++) {
		//glUniform3f(BillboardPosID, 200.0f + xtrans[i], 200.0f, 200.0f + ztrans[i]);
		glUniform3f(BillboardPosID, position.x, position.y, position.z);
		glUniform2f(BillboardSizeID, 0.1f, 0.1f);

		glEnableVertexAttribArray(posAttrib);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	glDisableVertexAttribArray(posAttrib);

	glUseProgram(0);
	GLSL::printError(__FILE__, __LINE__);
	assert(glGetError() == GL_NO_ERROR);
}

float Particle::randNum() {
    return ((float) rand() / (RAND_MAX)) * 600.0;
}

void Particle::setShaderProg(GLuint programID) {
	prog = programID;
    glUseProgram(prog);
    
    // Set up the shader variables
    posAttrib = glGetAttribLocation(prog, "position");
    uViewMatrix = glGetUniformLocation(prog, "V");
    uProjMatrix = glGetUniformLocation(prog, "P");
    uModelMatrix = glGetUniformLocation(prog, "M");

	// Shader variables for billboard
	CameraRight_worldspace_ID  = glGetUniformLocation(prog, "CameraRight_worldspace");
	CameraUp_worldspace_ID  = glGetUniformLocation(prog, "CameraUp_worldspace");
	BillboardPosID = glGetUniformLocation(prog, "BillboardPos");
	BillboardSizeID = glGetUniformLocation(prog, "BillboardSize");
	
	static constexpr GLfloat vertices[] = {
		/*-0.5f,  0.5f, // Top-left
		 0.5f,  0.5f, // Top-right
		 0.5f, -0.5f, // Bottom-right
		-0.5f, -0.5f  // Bottom-left
		*/
		/*-0.5, -0.5,
		-0.5, 0.5,
		0.5, 0.5, 
		0.5, -0.5*/
		/*-0.5f, -0.5f, 
		0.5f, -0.5f, 
		-0.5f, 0.5f, 
		0.5f, 0.5f,*/
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
}

#endif
