#ifndef PARTICLE_HPP
#define PARTICLE_HPP
#define NUM_PARTICLES 100

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "helper.h"
#include "GLSL.h"
#include "Camera.hpp"
#include <SOIL/SOIL.h>
#include <png.h>

class Particle {
	private:
		// Items necessary for shaders
		GLuint prog;
		GLuint tbo;
		GLuint vbo;
		GLuint ebo;

		GLuint texture;
		GLuint texSamplerID;
		const GLfloat vertices[8] = {
	   		 //  Position   Color             Texcoords
		    /*-0.5f,  0.5f, // Top-left
		     0.5f,  0.5f, // Top-right
		     0.5f, -0.5f, // Bottom-right
		    -0.5f, -0.5f  // Bottom-left
			*/
			/*-0.5, -0.5,
			-0.5, 0.5,
			0.5, 0.5, 
			0.5, -0.5*/
			-0.5, -0.5, 
			0.5, -0.5, 
			-0.5, 0.5, 
			0.5, 0.5
    	};

		const GLfloat texCoords[8] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f
		};

		const GLuint elements[6] = {
		    0, 1, 2,
		    3, 0, 2
		};

		GLuint CameraRight_worldspace_ID;
		GLuint CameraUp_worldspace_ID;
		GLuint BillboardPosID;
		GLuint BillboardSizeID;
		GLuint billboard_vertex_buffer;
		GLuint texture_vertex_buffer;

		GLuint posAttrib;
		GLuint texAttrib;
		GLuint colAttrib;

		GLuint uProjMatrix;
		GLuint uViewMatrix;
		// End list
		float xtrans[NUM_PARTICLES];
		float ytrans[NUM_PARTICLES];
		float ztrans[NUM_PARTICLES];

		//GLuint loadTexture();
		float randNum();
	public:
		Particle();
		void draw(Camera*);
};

Particle::Particle(GLuint program) {
	prog = program;
    glUseProgram(prog);
    
    // Set up the shader variables
    posAttrib = glGetAttribLocation(prog, "position");
	texAttrib = glGetAttribLocation(prog, "texcoord");
    uViewMatrix = glGetUniformLocation(prog, "V");
    uProjMatrix = glGetUniformLocation(prog, "P");

	// Shader variables for billboard
	CameraRight_worldspace_ID  = glGetUniformLocation(prog, "CameraRight_worldspace");
	CameraUp_worldspace_ID  = glGetUniformLocation(prog, "CameraUp_worldspace");
	BillboardPosID = glGetUniformLocation(prog, "BillboardPos");
	BillboardSizeID = glGetUniformLocation(prog, "BillboardSize");
	texSamplerID = glGetUniformLocation(prog, "myTextureSampler");
    
	int i;
	for (i = 0; i < NUM_PARTICLES; i++) {
		xtrans[i] = randNum();
		ytrans[i] = randNum();
		ztrans[i] = randNum();
	} 

/*
	texture = SOIL_load_OGL_texture("cloud.bmp",
									SOIL_LOAD_AUTO,
									SOIL_CREATE_NEW_ID,
									SOIL_FLAG_INVERT_Y);
	//texture = SOIL_load_OGL_texture // load an image file directly as a new OpenGL texture
    //(
    //    "cloud.png",
    //    SOIL_LOAD_AUTO,
    //    SOIL_CREATE_NEW_ID,
    //    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    //);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(texSamplerID, 0);
*/
    assert(glGetError() == GL_NO_ERROR);
}

void Particle::draw(Camera *camera) {
	glUseProgram(prog);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 view = camera->getViewMatrix();

	glUniform3f(CameraRight_worldspace_ID, view[0][0], view[1][0], view[2][0]);
	glUniform3f(CameraUp_worldspace_ID   , view[0][1], view[1][1], view[2][1]);

	// Set projection matrix
    glm::mat4 projection = camera->getProjectionMatrix();
    glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, glm::value_ptr(projection));

    // Set view matrix
    glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(view));

	// Send position array to GPU
	glEnableVertexAttribArray(posAttrib);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	

	int i;
	for (i = 0; i < NUM_PARTICLES; i++) {
		glUniform3f(BillboardPosID, 200.0f + xtrans[i], 100.0f, 200.0f + ztrans[i]);
		glUniform2f(BillboardSizeID, 20.0f, 20.0f);

		/*glEnableVertexAttribArray(posAttrib);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);*/
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

#endif
