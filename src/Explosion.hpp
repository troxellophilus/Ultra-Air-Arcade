#ifndef EXPLOSION_HPP
#define EXPLOSION_HPP
#define NUM_EXPLODE 60

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "helper.h"
#include "GLSL.h"
#include <png.h>

typedef struct Stage2 {
	int timeStep;
	bool toRender;
	float randFactor;
   glm::vec3 position;
   glm::vec3 velocity;
   glm::vec3 direction;
} Stage2;

float randFloat(float l, float h)
{
   float r = rand() / (float)RAND_MAX;
   return (1.0f - r) * l + r * h;
}


class Explosion {
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
		Stage2 particles[NUM_EXPLODE];

		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		glm::mat4 rotMatrix;
		glm::vec3 position;
      glm::vec3 velocity;
		glm::quat quaternion;
      float h;
      float damp;

		float randNum();
	public:
		Explosion();
		bool occuring;
      void update(glm::mat4, glm::mat4);
      void setPosition(glm::vec3 pos);
		void draw(float);
		void setShaderProg(GLuint);
};

Explosion::Explosion() : 
   occuring(false),
   h(0.0)
{
	int i;
	for (i = 0; i < NUM_EXPLODE; i++) {
		particles[i].timeStep = NUM_EXPLODE - 1;
		particles[i].toRender = false;
		particles[i].randFactor = randNum();
	} 

	particleIndex = 0;
	frameCount = 0;
}

void Explosion::setPosition(glm::vec3 pos){
   position = pos;
   velocity = glm::vec3(randFloat(1.0f, 1.0f), randFloat(1.0f, 1.0f), randFloat(1.0f, 1.0f));
  
   for(int i = 0; i < NUM_EXPLODE; i++){
      particles[i].position = pos;
      particles[i].velocity = glm::vec3(randFloat(0.0f, 1.0f), randFloat(0.0f, 1.0f), randFloat(0.0f, 1.0f));
   }
}

void Explosion::update(glm::mat4 viewMat, glm::mat4 projMat) {
	viewMatrix = viewMat;
	projMatrix = projMat;


	if (frameCount < NUM_EXPLODE) particles[frameCount++].toRender = true;
}

void Explosion::draw(float thrust) {
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
   
   velocity = velocity + .01f * glm::vec3(0.0, -0.4, 0.0); 
	position = position + .01f * velocity;

   for (i = 0; i < NUM_EXPLODE; i++) {
		if (particles[i].toRender) {
			float factor = (-thrust) * particles[i].timeStep * particles[i].randFactor * 0.07 / NUM_EXPLODE;
			//glm::vec3 direction = (0.15f + factor) * glm::normalize(glm::vec3(0, -0.25, 1) * glm::inverse(quaternion));
         
         particles[i].velocity = particles[i].velocity + .01f * glm::vec3(-0.4, -0.4, -0.4);
         particles[i].position = particles[i].position + .01f * particles[i].velocity;         

         glUniform3f(BillboardPosID, particles[i].position.x, particles[i].position.y, particles[i].position.z);
			glUniform1i(TimeStepID, particles[i].timeStep);

			if (particles[i].timeStep == NUM_EXPLODE -1){
            particles[i].timeStep = 0;
			}
         else{
            particles[i].timeStep++;
         }

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	   // cout << "Particles: "  << particles[i].timeStep << endl;
   }

	glDisableVertexAttribArray(posAttrib);

	glUseProgram(0);
	GLSL::printError(__FILE__, __LINE__);
	assert(glGetError() == GL_NO_ERROR);
}

float Explosion::randNum() {
    return ((float) rand() / (RAND_MAX)) * 1.005;
}

void Explosion::setShaderProg(GLuint programID) {
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
	
	static GLfloat vertices[] = {
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
