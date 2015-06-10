/*
 * Ultra Air Arcade Code Base
 * CPE 476
 */

#include <GL/glew.h>

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#endif

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define CELL 32

#define SHADOWS_OFF '1'
#define SHADOW_ON '2'

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Entity.hpp"
#include "Collision.hpp"
#include "Camera.hpp"
#include "Materials.hpp"
#include "Terrain.h"
#include "Rules.hpp"
#include "Projectile.hpp"
#include "Skybox.hpp"
#include "RacerAI.hpp"
#include "Scenery.hpp"
#include "DrawText.h"
#include "Frustum.h"
#include "PlaneSound.hpp"

#include "helper.h"
#include "GLSL.h"
#include "GLSLProgram.h"

//#define DEBUG
#define NUM_OPPONENTS 15

#include <string>
#include <iostream>
#include <vector>
#include <cmath>

//#define _DEBUG
#define NUM_PLAYER_MATS 5

#define GLDEBUG assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));

using namespace std;
//using namespace glm;

enum { PLANE, MISSILE, CHECKPOINT, TERRAIN, ROCK, STONE1, STONE2, STONE3, STONE4, STONE5, TREE, NUM_VBO };

//Shader IDs
GLuint passThroughShaders;
GLuint depthCalcShaders;
GLuint renderSceneShaders;
GLuint skyBoxShaders;
GLuint textShaders;
GLuint propShaders;
GLuint particleShaders;

GLFWwindow* window;

//Various data buffers
GLuint vao;
GLuint pbo[NUM_VBO];
GLuint nbo[NUM_VBO];
GLuint ibo[NUM_VBO];
GLuint tbo[NUM_VBO];

//common glsl variable ids
GLint aPos = 0;
GLint aNor = 0;
GLint lPos = 0;
GLint uViewMatrix = 0;
GLint uModelMatrix = 0;
GLint uProjMatrix = 0;
GLint renderObj = 0;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;

// Shadow variables
GLuint fbo;
GLuint depthMatrixID;
GLuint depthTexture;
GLuint depthBiasID;
GLuint shadowMapID;

glm::vec3 lightPosition = glm::vec3(256.0f, 1000.0f, 256.0f);
glm::mat4 biasMatrix = glm::mat4(
								 0.5, 0.0, 0.0, 0.0,
								 0.0, 0.5, 0.0, 0.0,
								 0.0, 0.0, 0.5, 0.0,
								 0.5, 0.5, 0.5, 1.0
								 );
glm::mat4 depthBiasMVP;
glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
glm::mat4 depthViewMatrix;
glm::mat4 depthModelMatrix;
glm::mat4 depthMVP;

float lightX = lightPosition.x, lightY = lightPosition.y, lightZ = lightPosition.z;

// For renderscene and shadowmap shaders
GLuint shadowPos = 0;
GLuint shadowMapPos;
GLuint shadowNor = 0;
GLuint shadowLPos;
GLuint shadowViewMatrix;
GLuint shadowModelMatrix;
GLuint shadowProjMatrix;

//for props
GLuint pPos;
GLuint pNor;
GLuint propV;
GLuint propM;
GLuint propP;
GLuint prop_light_position;
GLuint propTexture;
GLuint tbo_tex;
// End

// HUD vars
GLint h_aCoord;
GLint h_uText;
GLint h_uColor;

vector<float> terPosBuf;
vector<float> terNorBuf;
vector<unsigned int> terIndBuf;

Object obj[NUMSHAPES];

Collision collision = Collision();
PlaneSound planeSound = PlaneSound("../Assets/sound/JetEngine.wav");
PlaneSound backgroundMusic = PlaneSound("../Assets/sound/destiny-short.wav");
PlaneSound collisionSound = PlaneSound("../Assets/sound/explosion-01.wav");

Terrain terrain = Terrain();
Skybox *skybox;
DrawText *drawText;

Rules rules = Rules();
RacerAI playerAI = RacerAI();
Camera camera = Camera();
Entity player = Entity(&playerAI);
Frustum viewFrustum = Frustum();

vector<Entity> opponents;
vector<Entity> opp_props;

static float g_width, g_height;

unsigned int numObj = 0;
unsigned int collisions = 0;
unsigned int pIndices = 0;
unsigned int whichShader = 0;
float missleTime;

bool collisionDetectedTerrain = false;
bool collisionDetectedOpponent = false;
bool renderShadows;
int collisionCount = 0;

bool beginProjectile = false;
Projectile *missle;
Entity projectileEntity = Entity();
float start;
float elapsed;

Material p_mat_list[NUM_PLAYER_MATS] = {
	Materials::brass,
	Materials::turquoise,
	Materials::pink,
	Materials::emerald,
	Materials::jade
};

float randNum() {
	return ((float) rand() / (RAND_MAX)) * 600.0;
}

inline float clamp(float value, float minNum, float maxNum)
{
	return min(max(minNum, value), maxNum);
}

int initVBO(Entity *e, int i, bool textures);

// EVENT CALLBACKS
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	static int mat_idx = 0;
	// Check escape key
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	// Check movement keys
	if (action == GLFW_REPEAT || action == GLFW_PRESS) {
		// Check Game State
		if (action == GLFW_PRESS && rules.getState() == Rules::SPLASH) {
			rules.setState(Rules::CSEL);
		}
		else if (action == GLFW_PRESS && rules.getState() == Rules::CSEL) {
			if (key == GLFW_KEY_RIGHT)
				mat_idx++;
			else if (key == GLFW_KEY_LEFT)
				mat_idx--;
			else
				rules.setState(Rules::SETUP);
			
			if (mat_idx < 0)
				mat_idx = NUM_PLAYER_MATS - 1;
			if (mat_idx == NUM_PLAYER_MATS)
				mat_idx = 0;
			
			player.setMaterial(p_mat_list[mat_idx]);
			player.setBaseMaterial(p_mat_list[mat_idx]);
		}
		else if (action == GLFW_PRESS && rules.getState() == Rules::FINISH) {
			rules.setState(Rules::LEADERBOARD);
		}
		
		else if (action == GLFW_PRESS && rules.getState() == Rules::LEADERBOARD) {
			rules.setState(Rules::SPLASH);
		}
		else if (rules.getState() == Rules::RACE) {
			// Check movement keys
			if (key == GLFW_KEY_W) {
				camera.move(Camera::FORWARD);
				planeSound.changePitch(1);
				player.throttleUp();
			}
			if (key == GLFW_KEY_S) {
				camera.move(Camera::BACK);
				planeSound.changePitch(-1);
				player.throttleDown();
			}
			if (key == GLFW_KEY_A) {
				camera.move(Camera::LEFT);
				player.rollLeft();
			}
			if (key == GLFW_KEY_D) {
				camera.move(Camera::RIGHT);
				player.rollRight();
			}
			if (mods == GLFW_MOD_SHIFT) {
				camera.move(Camera::DOWN);
			}
			if (key == GLFW_KEY_SPACE) {
				camera.move(Camera::UP);
			}
			if (key == GLFW_KEY_M) {
				if (camera.getMode() == Camera::TPC)
					camera.setMode(Camera::FREE);
				else
					camera.setMode(Camera::TPC);
			}
			
			if (action == GLFW_PRESS && key == GLFW_KEY_X) {
				if (!beginProjectile) {
					beginProjectile = true;
					missle = new Projectile(projectileEntity, false, true, player.getPosition(), opponents[1].getPosition());
					missleTime = elapsed;
					//cout << "MISSILE TIME: " << missleTime << endl;
					//cout << "New Projectile" << endl;
				}
			}
			
			if (key == GLFW_KEY_P) {
				camera.setPlayer(&player);
			}
			if (key == GLFW_KEY_O) {
				camera.setPlayer(&(opponents[0]));
			}
			
			if (key == GLFW_KEY_N) {
				printf("glm::vec3(%f, %f, %f),\n", player.getPosition().x, player.getPosition().y, player.getPosition().z);
			}
		}
	}
}

static void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
	static double last_xpos = xpos;
	static double last_ypos = ypos;
	
	player.pitch(float(ypos - last_ypos));
	camera.pitch(float(ypos - last_ypos));
	
	player.turn(float(xpos - last_xpos));
	camera.yaw(float(xpos - last_xpos));
	
	last_xpos = xpos;
	last_ypos = ypos;
}

/* model transforms */
void SetModel(vec3 trans, glm::vec3 rot, vec3 sc) {
	glm::mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
	glm::mat4 Orient = glm::orientation(rot, glm::vec3(0, 1, 0));
	glm::mat4 Sc = glm::scale(glm::mat4(1.0f), sc);
	glm::mat4 com = Trans * Orient * Sc;
	glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(com));
}

glm::mat4 SetModel(vec3 trans, glm::vec3 rot, vec3 sc, int num) {
	glm::mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
	glm::mat4 Orient = glm::orientation(rot, glm::vec3(0, 1, 0));
	glm::mat4 Sc = glm::scale(glm::mat4(1.0f), sc);
	glm::mat4 com = Trans * Orient * Sc;
	return com;
}

int initVBO(Entity *e, int whichbo, bool textures = false) {
	vector<float> posBuf, norBuf, texBuf;
	vector<unsigned int> indBuf;
	if(textures)
		e->packVertices(&posBuf, &norBuf, &indBuf, &texBuf);
	else
		e->packVertices(&posBuf, &norBuf, &indBuf);
	
	// Send the position array to the GPU
	glGenBuffers(1, &pbo[whichbo]);
	glBindBuffer(GL_ARRAY_BUFFER, pbo[whichbo]);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	glGenBuffers(1, &nbo[whichbo]);
	glBindBuffer(GL_ARRAY_BUFFER, nbo[whichbo]);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	
	// Send the index array to the GPU
	glGenBuffers(1, &ibo[whichbo]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[whichbo]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
	
	if(textures)
	{
		// Generate a texture buffer object
		glGenBuffers(1, &tbo[whichbo]);
		GLDEBUG
		// Bind the current texture to be the newly generated texture object
		
		glBindBuffer(GL_TEXTURE_BUFFER, tbo[whichbo]);
		glBufferData(GL_TEXTURE_BUFFER, sizeof(tbo[whichbo]), &tbo[whichbo], GL_STATIC_DRAW);
		
		glGenTextures(1, &tbo_tex);
		
		// Unbind
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	return (int)indBuf.size();
}

void initGround() {
	terrain = Terrain("../Assets/heightmap/UltraAirArcade.bmp", 50.0, terPosBuf, terIndBuf, terNorBuf);
	
	glGenBuffers(1, &pbo[TERRAIN]);
	glBindBuffer(GL_ARRAY_BUFFER, pbo[TERRAIN]);
	glBufferData(GL_ARRAY_BUFFER, terPosBuf.size()*sizeof(float), &terPosBuf[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &nbo[TERRAIN]);
	glBindBuffer(GL_ARRAY_BUFFER, nbo[TERRAIN]);
	glBufferData(GL_ARRAY_BUFFER, terNorBuf.size()*sizeof(float), &terNorBuf[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &ibo[TERRAIN]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[TERRAIN]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, terIndBuf.size()*sizeof(unsigned int), &terIndBuf[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void initCollisions() {
	collision = Collision(&terrain, &player, &opponents);
	
	planeSound.playLooped();
	
	backgroundMusic.setVolume(50.f);
	backgroundMusic.playLooped();
	
	collision.setPlayerSound(&planeSound);
	collision.setCollisionSound(&collisionSound);
}

void initShaderVars() {
	// Set up the shader variables
	GLDEBUG
	// Set up the shader variables
	pPos = glGetAttribLocation(propShaders, "prop_position");
	pNor = glGetAttribLocation(propShaders, "prop_normal");
	
	propV = glGetUniformLocation(propShaders, "V");
	propM = glGetUniformLocation(propShaders, "M");
	propP = glGetUniformLocation(propShaders, "P");
	prop_light_position = glGetUniformLocation(propShaders, "lPos");
	propTexture = glGetUniformLocation(propShaders, "tex_un");
	
	aPos = glGetAttribLocation(passThroughShaders, "aPos");
	aNor = glGetAttribLocation(passThroughShaders, "aNor");
	shadowPos = glGetAttribLocation(renderSceneShaders, "aPos");
	shadowNor = glGetAttribLocation(renderSceneShaders, "aNor");
	shadowMapPos = glGetAttribLocation(depthCalcShaders, "aPos");
	
	uViewMatrix = glGetUniformLocation(passThroughShaders, "V");
	uModelMatrix = glGetUniformLocation(passThroughShaders, "M");
	uProjMatrix = glGetUniformLocation(passThroughShaders, "P");
	lPos = glGetUniformLocation(passThroughShaders, "lPos");
	renderObj = glGetUniformLocation(passThroughShaders, "renderObj");
	
	h_uMatAmb = glGetUniformLocation(passThroughShaders, "UaColor");
	h_uMatDif = glGetUniformLocation(passThroughShaders, "UdColor");
	h_uMatSpec = glGetUniformLocation(passThroughShaders, "UsColor");
	h_uMatShine = glGetUniformLocation(passThroughShaders, "Ushine");
	
	depthBiasID =    glGetUniformLocation(renderSceneShaders, "depthBiasMVP");
	shadowMapID =  glGetUniformLocation(renderSceneShaders, "shadowMap");
	shadowViewMatrix = glGetUniformLocation(renderSceneShaders, "V");
	shadowModelMatrix = glGetUniformLocation(renderSceneShaders, "M");
	shadowProjMatrix = glGetUniformLocation(renderSceneShaders, "P");
	shadowLPos = glGetUniformLocation(renderSceneShaders, "lPos");
	depthMatrixID = glGetUniformLocation(depthCalcShaders, "depthMVP");
	
	
	
	if (renderShadows) {
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
		glDrawBuffer(GL_NONE);
	}
	
	assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
}

GLuint installShaders(const string &vShaderName, const string &fShaderName) {
	GLuint prog;
	GLint rc;
	GLSL::printError(__FILE__, __LINE__);
	
	// Create Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	// Create shader handles
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
	GLSL::printError(__FILE__, __LINE__);
	
	// Read shader sources
	const char *vshader = GLSL::textFileRead(vShaderName.c_str());
	const char *fshader = GLSL::textFileRead(fShaderName.c_str());
	glShaderSource(VS, 1, &vshader, NULL);
	glShaderSource(FS, 1, &fshader, NULL);
	GLSL::printError(__FILE__, __LINE__);
	
	// Compile vertex shader
	glCompileShader(VS);
	GLSL::printError(__FILE__, __LINE__);
	glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(VS);
	if (!rc) {
		printf("Error compiling vertex shader %s\n", vShaderName.c_str());
		return false;
	}
	
	// Compile fragment shader
	glCompileShader(FS);
	GLSL::printError(__FILE__, __LINE__);
	glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(FS);
	if (!rc) {
		printf("Error compiling fragment shader %s\n", fShaderName.c_str());
		return false;
	}
	
	// Create the program and link
	prog = glCreateProgram();
	glAttachShader(prog, VS);
	glAttachShader(prog, FS);
	
	if (whichShader == 0)
		glBindFragDataLocation(prog, 0, "fragmentDepth");
	else
		glBindFragDataLocation(prog, 0, "outColor");
	
	glLinkProgram(prog);
	GLSL::printError(__FILE__, __LINE__);
	glGetProgramiv(prog, GL_LINK_STATUS, &rc);
	GLSL::printProgramInfoLog(prog);
	if (!rc) {
		printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
		return false;
	}
	
	glUseProgram(prog);
	assert(glGetError() == GL_NO_ERROR);
	return prog;
}

void SetMaterial(Material mat) {
	glUseProgram(passThroughShaders);
	
	glm::vec3 amb = mat.getAmbient();
	glm::vec3 dif = mat.getDiffuse();
	glm::vec3 spc = mat.getSpecular();
	float shn = mat.getShininess();
	
	glUniform3f(h_uMatAmb, amb.x, amb.y, amb.z);
	glUniform3f(h_uMatDif, dif.x, dif.y, dif.z);
	glUniform3f(h_uMatSpec, spc.x, spc.y, spc.z);
	glUniform1f(h_uMatShine, shn);
}

void drawPassthrough(Entity* entity, int nIndices, int whichbo)
{
	glEnableVertexAttribArray(aPos);
	glBindBuffer(GL_ARRAY_BUFFER, pbo[whichbo]);
	glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Enable and bind normal array for drawing
	glEnableVertexAttribArray(aNor);
	glBindBuffer(GL_ARRAY_BUFFER, nbo[whichbo]);
	glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Bind index array for drawing
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[whichbo]);
	
	glUniform3f(lPos, lightPosition.x, lightPosition.y, lightPosition.z);
	
	glUniform1i(renderObj, 1);
	
	if (whichbo == CHECKPOINT)
		glUniform1i(renderObj, 3);
	
	SetMaterial(entity->getMaterial());
	glm::mat4 Trans = glm::translate( glm::mat4(1.0f), entity->getPosition());
	glm::mat4 Orient = entity->getRotationM();
	glm::mat4 Sc = glm::scale(glm::mat4(1.0f), entity->getScale());
	glm::mat4 com = Trans * Orient * Sc;
	glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(com));
	assert(glGetError() == GL_NO_ERROR);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
	assert(glGetError() == GL_NO_ERROR);
	// Disable and unbind
	GLSL::disableVertexAttribArray(aPos);
	GLSL::disableVertexAttribArray(aNor);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void drawProp(Entity* entity, int nIndices, int whichbo)
{
	
	
	glEnableVertexAttribArray(pPos);
	glBindBuffer(GL_ARRAY_BUFFER, pbo[whichbo]);
	glVertexAttribPointer(pPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Enable and bind normal array for drawing
	glEnableVertexAttribArray(pNor);
	glBindBuffer(GL_ARRAY_BUFFER, nbo[whichbo]);
	glVertexAttribPointer(pNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	GLDEBUG
	
	
	// Bind index array for drawing
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[whichbo]);
	
	glUniform3f(prop_light_position, lightPosition.x, lightPosition.y, lightPosition.z);
	
	//bind textures
	glActiveTexture(GL_TEXTURE0);
	GLDEBUG
	glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
	GLDEBUG
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, tbo[whichbo]);
	GLDEBUG
	glUniform1i(propTexture,0);
	GLDEBUG
	
	
	
	glm::mat4 Trans = glm::translate( glm::mat4(1.0f), entity->getPosition());
	glm::mat4 Orient = entity->getRotationM();
	glm::mat4 Sc = glm::scale(glm::mat4(1.0f), entity->getScale());
	glm::mat4 com = Trans * Orient * Sc;
	glUniformMatrix4fv(propM, 1, GL_FALSE, glm::value_ptr(com));
	
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
	
	
	// Disable and unbind
	GLSL::disableVertexAttribArray(pNor);
	GLSL::disableVertexAttribArray(pPos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawVBO(Entity *entity, int nIndices, int whichbo, GLuint shader = passThroughShaders) {
	glUseProgram(shader);
	
	if(shader == passThroughShaders)
		drawPassthrough(entity, nIndices, whichbo);
	else if(shader == propShaders)
		drawProp(entity,nIndices,whichbo);
	
	glUseProgram(0);
	GLDEBUG
}

void drawGround(glm::mat4 projMatrix, glm::mat4 viewMatrix) {
	if (!renderShadows) {
		glUseProgram(passThroughShaders);
		glEnable(GL_CULL_FACE);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glEnableVertexAttribArray(aPos);
		glBindBuffer(GL_ARRAY_BUFFER, pbo[TERRAIN]);
		glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		GLSL::enableVertexAttribArray(aNor);
		glBindBuffer(GL_ARRAY_BUFFER, nbo[TERRAIN]);
		glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[TERRAIN]);
		
		glUniform1i(renderObj, 0);
		SetMaterial(Materials::wood);
		SetModel(glm::vec3(0), glm::vec3(0, 1, 0), vec3(1));
		glDrawElements(GL_TRIANGLES, (int)terIndBuf.size(), GL_UNSIGNED_INT, 0);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		GLSL::disableVertexAttribArray(aPos);
		GLSL::disableVertexAttribArray(aNor);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glUseProgram(0);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
	} else {
		glUseProgram(depthCalcShaders);
		depthModelMatrix = SetModel(glm::vec3(0), glm::vec3(0, 1, 0), vec3(1), 1);
		depthViewMatrix = glm::lookAt(lightPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		
		depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
		glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, glm::value_ptr(depthMVP));
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glEnableVertexAttribArray(shadowMapPos);
		glBindBuffer(GL_ARRAY_BUFFER, pbo[TERRAIN]);
		glVertexAttribPointer(shadowMapPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[TERRAIN]);
		glDrawElements(GL_TRIANGLES, (int)terIndBuf.size(), GL_UNSIGNED_INT, 0);
		GLSL::disableVertexAttribArray(shadowMapPos);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		// Done drawing shadow map
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, g_width, g_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(renderSceneShaders);
		glUniform3f(shadowLPos, lightPosition.x, lightPosition.y, lightPosition.z);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glm::mat4 projection = projMatrix;
		glUniformMatrix4fv(shadowProjMatrix, 1, GL_FALSE, glm::value_ptr(projection));
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		// Set view matrix
		glm::mat4 view = viewMatrix;
		glUniformMatrix4fv(shadowViewMatrix, 1, GL_FALSE, glm::value_ptr(view));
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glm::mat4 model = SetModel(glm::vec3(0), glm::vec3(0, 1, 0), vec3(1), 1);
		glUniformMatrix4fv(shadowModelMatrix, 1, GL_FALSE, glm::value_ptr(model));
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(shadowMapID, 0);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glEnableVertexAttribArray(shadowPos);
		glBindBuffer(GL_ARRAY_BUFFER, pbo[TERRAIN]);
		glVertexAttribPointer(shadowPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		GLSL::enableVertexAttribArray(shadowNor);
		glBindBuffer(GL_ARRAY_BUFFER, nbo[TERRAIN]);
		glVertexAttribPointer(shadowNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[TERRAIN]);
		glDrawElements(GL_TRIANGLES, (int)terIndBuf.size(), GL_UNSIGNED_INT, 0);
		
		GLSL::disableVertexAttribArray(shadowPos);
		GLSL::disableVertexAttribArray(shadowNor);
	}
	assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
}

void drawHUD(int fps, float pitch) {
	char buffer[256], *text;
	
	drawText->addText(Text(".", g_width / 2, g_height / 2, 0, 3, drawText->getFontSize(45), 2));
	drawText->addText(Text("_______                  _______", g_width / 2 - g_width / 4, g_height / 2 + pitch, 0, 1, drawText->getFontSize(45), 2));
	
	// drawText->addText(Text(" _______", 0.125 * g_width, 0.575 * g_height, 0, 1, drawText->getFontSize(90), 2));
	// drawText->addText(Text("|_______|", 0.125 * g_width, 0.55 * g_height, 0, 1, drawText->getFontSize(90), 2));
	
	// snprintf(buffer, sizeof(buffer), "%.0f", fabs(player.getVelocity().x + player.getVelocity().y + player.getVelocity().z) * 10);
	// int len = strlen(buffer) + 1;
	// text = new char[len];
	// strncpy(text, buffer, len);
	// drawText->addText(Text(text, 0.145 * g_width, 0.55 * g_height, 0, 0, drawText->getFontSize(90), 1));
	
	// drawText->addText(Text("1st", 0.05 * g_width, 0.1 * g_height, 0, 0, drawText->getFontSize(30), 1));
	// drawText.addText(Text("Thrust: 25%", 0.025 * g_width, 0.98 * g_height, 0, 0, drawText.getFontSize(90), 1));
	// drawText.addText(Text("Weapon: Gun", 0.025 * g_width, 0.96 * g_height, 0, 0, drawText.getFontSize(90), 1));
	
	// snprintf(buffer, sizeof(buffer), "Collision Count: %d", collisionCount);
	// len = strlen(buffer) + 1;
	// text = new char[len];
	// strncpy(text, buffer, len);
	// drawText->addText(Text(text, 0.025 * g_width, 0.96 * g_height, 0, 0, drawText->getFontSize(45), 1));
	
	// drawText->addText(Text(" _______", 0.8 * g_width , 0.575 * g_height, 0, 1, drawText->getFontSize(90), 2));
	// drawText->addText(Text("|_______|", 0.8 * g_width , 0.55 * g_height, 0, 1, drawText->getFontSize(90), 2));
	
	// snprintf(buffer, sizeof(buffer), "%.0fm", player.getPosition().y * 10);
	// len = strlen(buffer) + 1;
	// text = new char[len];
	// strncpy(text, buffer, len);
	// drawText->addText(Text(text, 0.81 * g_width, 0.55 * g_height, 0, 0, drawText->getFontSize(90), 1));
	
	// snprintf(buffer, sizeof(buffer), "FPS: %d", fps);
	// drawText->addText(Text(buffer, 0.9 * g_width, 0.95 * g_height, 0, 0, drawText->getFontSize(90), 1));
	
	// glUniform1i(renderObj, 2);
	glDisable(GL_CULL_FACE);
	glUseProgram(textShaders);
	drawText->drawText();
	glEnable(GL_CULL_FACE);
}

int main(int argc, char **argv) {
	const GLubyte* renderer;
	const GLubyte* version;
	int indices[7];
	
	srand(time(NULL));
	
	if (argc > 1) {
		argv++;
		if (**argv == SHADOWS_OFF) renderShadows = false;
		else renderShadows = true;
	} else renderShadows = true;
	
	GLFWwindow* window;
	
	glfwSetErrorCallback(error_callback);
	
	// Initialise GLFW
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	
	//glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	g_width = 1024;
	g_height = 768;
	
	// Open a window and create its OpenGL context
	window = glfwCreateWindow(g_width, g_height, "Ultra Air Arcade | alpha build", NULL, NULL);
	if ( window == NULL ) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	GLSLProgram::checkForOpenGLError(__FILE__, __LINE__);
	// Set key and cursor callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Load 3D models
	loadShapes("../Assets/models/sphere.obj", obj[0]);
	loadShapes("../Assets/models/cube.obj", obj[1]);
	loadShapes("../Assets/models/Pyro.obj", obj[2]);
	loadShapes("../Assets/models/mig.obj", obj[3]);
	loadShapes("../Assets/models/missile.obj", obj[4]);
	loadShapes("../Assets/models/Rock.obj", obj[5]);
	loadShapes("../Assets/models/stone1.obj", obj[6]);
	loadShapes("../Assets/models/stone2.obj", obj[7]);
	loadShapes("../Assets/models/stone3.obj", obj[8]);
	loadShapes("../Assets/models/stone4.obj", obj[9]);
	loadShapes("../Assets/models/stone5.obj", obj[10]);
	loadShapes("../Assets/models/tree.obj", obj[11]);
	std::cout << " loaded the objects " << endl;
	
	
	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if ( glewInit() != GLEW_OK ) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	
	GLSLProgram::checkForOpenGLError(__FILE__, __LINE__);
	// Print opengl version & GPU info
	renderer = glGetString (GL_RENDERER);
	version = glGetString (GL_VERSION);
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported: %s\n", version);
	
	assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
	/* tell GL to only draw onto a pixel if the shape is closer to the viewer */
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
	
	depthCalcShaders = installShaders("shd/depthmap_vert.glsl", "shd/depthmap_frag.glsl");
	whichShader = 1;
	renderSceneShaders = installShaders("shd/renderscene_vert.glsl", "shd/renderscene_frag.glsl");
	passThroughShaders = installShaders("shd/basic.vert", "shd/basic.frag");
	skyBoxShaders = installShaders("shd/skybox_vert.glsl", "shd/skybox_frag.glsl");
	textShaders = installShaders("shd/text.v.glsl", "shd/text.f.glsl");
	propShaders = installShaders("shd/propShader.vert", "shd/propShader.frag");
	particleShaders = installShaders("shd/particle_vert.glsl", "shd/particle_frag.glsl");
	
	initShaderVars();
	skybox = new Skybox(skyBoxShaders);
	skybox->initShaderVars();
	
	drawText = new DrawText(textShaders);
	drawText->initResources(g_width, g_height);
	
	initCollisions();
	initGround();
	assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
	
	/*
	 for(int i = 0; i < 7; i++){
	 Entity dumb;
	 dumb.setObject(&obj[i+5]);
	 indices[i] = initVBO(&dumb, ROCK + i, true);
	 GLDEBUG
	 }
	 */
	
	/*
	 std::vector<int> typeProp;
	 std::vector<Entity> props = generateScenery(typeProp, obj, &terrain);
	 */
	
	// Initialize player
	playerAI.setType(RacerAI::PLAYER);
	player.setType(PLAYER_ENTITY);
	player.setObject(&obj[3]);
	player.setPosition(glm::vec3(170.056442, 13.324112, 222.544495));
	player.setScale(glm::vec3(0.25, 0.25, 0.25));
	player.setMaterial(p_mat_list[0]);
	player.setBaseMaterial(p_mat_list[0]);
	player.setParticleProg(particleShaders);
	player.calculateBoundingSphereRadius();
	pIndices = initVBO(&player, PLANE);
	
	// Initialize camera
	camera.setMode(Camera::TPC);
	camera.setPosition(glm::vec3(0, 0, -10));
	camera.setClipping(0.1, 1500);
	camera.setFOV(90);
	camera.setPlayer(&player);
	
	//cout << "Init\n";
	projectileEntity.setObject(&obj[4]);
	projectileEntity.setPosition(glm::vec3(player.getPosition()[0], player.getPosition()[1], player.getPosition()[2]));
	projectileEntity.setScale(glm::vec3(2.0, 2.0, 2.0));
	projectileEntity.setMaterial(Materials::emerald);
	int mIndices = initVBO(&projectileEntity, MISSILE);
	projectileEntity.calculateBoundingSphereRadius();
	
	// Initialize opponents
	RacerAI *ai;
	Entity opp;
	//Entity prop;
	int odx = 1;
	while (odx <= NUM_OPPONENTS) {
		ai = new RacerAI();
		//printf("ai: %llu\n", (uint64_t)ai);
		opp = Entity(ai);
		opp.setType(AI_ENTITY);
		opp.setObject(&obj[3]);
		//glm::vec3 epos = player.getPosition();
		//opp.setPosition(glm::vec3(epos.x + odx * 0.7f, epos.y, epos.z + odx * 0.4f));
		opp.setScale(glm::vec3(0.25, 0.25, 0.25));
		opp.calculateBoundingSphereRadius();
		opp.setMaterial(p_mat_list[odx % NUM_PLAYER_MATS]);
		opp.setBaseMaterial(p_mat_list[odx % NUM_PLAYER_MATS]);
		opp.setParticleProg(particleShaders);
		
		opponents.push_back(opp);
		odx++;
	}
	
	// Initialize collisions
	collision.setOpponents(&opponents);
	
	// Initialize game rules
	rules.setAgents(&opponents);
	rules.setPlayer(&player);
	
	RacerAI *propAI = new RacerAI();
	
	// Initialize Props
	vector<Entity> checkpoints;
	int c = 0;
	while (c < TRACK_LOCS) {
		propAI->setType(RacerAI::PROP);
		Entity prop = Entity(propAI);
		prop.setType(PROP_ENTITY);
		prop.setObject(&obj[0]);
		prop.setScale(glm::vec3(2., 2., 2.));
		prop.setPosition(global_track[c]);
		checkpoints.push_back(prop);
		c++;
	}
	int cIndices = initVBO(&checkpoints[0], CHECKPOINT);
	
	// Initialize rules
	rules.setPlayer(&player);
	rules.setAgents(&opponents);
	
	unsigned int frames = 0;
	double lastTime = glfwGetTime();
	int fps = 0;
	
	float start = glfwGetTime();
	elapsed = 0;
	float last = -1;
	float pitch = 0;
	
	glm::quat rol = glm::angleAxis(-0.15f, glm::vec3(0, 0, 1));
	
	//Projectile pathPlane = Projectile(bigOpp, true, bigOpp.getPosition(), bigOpp.getPosition());
	bool color = false;
	
	// Frame loop
	while (!glfwWindowShouldClose(window)) {
		float ratio;
		int width, height;
		
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		
		// Set projection matrix
		glm::mat4 projection = camera.getProjectionMatrix();
		
		glm::mat4 view = camera.getViewMatrix();
		
		
		glUseProgram(passThroughShaders);
		glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(view));
		
		glUseProgram(propShaders);
		glUniformMatrix4fv(propP, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(propV, 1, GL_FALSE, glm::value_ptr(view));
		
		// Set View Frustum
		viewFrustum.setFrustum(view, projection);
		
		drawGround(projection, view);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		// Update the rules and game state
		rules.update(&camera);
		
		// Update the collisions if in race state
		if (rules.getState() == Rules::RACE) {
			collision.update();
		}
		
		// Update & draw player
		player.update();
		player.update(view, projection);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		drawVBO(&player, pIndices, PLANE);
      	player.drawExhaust();
		//checkPlayerCollisions();
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		if (player.getPitch() > 0) {
			if (pitch > 0) {
				pitch -= 0.4;
			}
			else {
				pitch -= 0.2;
			}
		}
		else if (player.getPitch() < 0) {
			if (pitch > 0) {
				pitch += 0.4;
			}
			else {
				pitch += 0.8;
			}
		}
		else {
			if (pitch > 0) {
				pitch -= 0.2;
			}
			else if (pitch < 0) {
				pitch += 0.2;
			}
		}
		
		
		int i = 0, t = 0;
		// Update & draw opponents
		for (auto &opponent : opponents) {
			t++;
			opponent.update();
			opponent.update(view, projection);
			if (viewFrustum.sphereInFrustum(opponent.getPosition(), opponent.getRadius())) {
				drawVBO(&opponent, pIndices, PLANE);
            	opponent.drawExhaust();
				i++;
			}
			//checkOpponentCollisions(opponent);
			assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		}
		
		/*
		 //update scenery
		 for(int i = 0; i < props.size(); i++){
		 //cout << indices[typeProp[i]] << endl;
		 drawVBO(&props[i], indices[typeProp[i]], ROCK + typeProp[i], propShaders);
		 }
		 */
		
		//update skybox
		skybox->render(view, projection, camera.getPosition());
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		//update checkpoints
		Entity p_cp = checkpoints[((RacerAI *)player.getAI())->getTrackIdx()];
		if (viewFrustum.sphereInFrustum(p_cp.getPosition(), p_cp.getRadius()))
			drawVBO(&p_cp, cIndices, CHECKPOINT);
		/*
		 for (auto &checkpoint : checkpoints) {
		 if (viewFrustum.sphereInFrustum(checkpoint.getPosition(), checkpoint.getRadius())) {
		 drawVBO(&checkpoint, cIndices, CHECKPOINT);
		 }
		 }
		 */
		
		// Update camera
		camera.update();
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		/*
		 //projectilw handling
		 if (beginProjectile == true) {
		 //cout << "MISSILE TIME: " << missleTime << endl;
		 //cout << "ELAPSED: " << elapsed << endl;
		 
		 int isDone = missle->runProjectile(elapsed - missleTime,
		 player.getPosition(), opponents[0].getPosition());
		 
		 Entity* ent = missle->getEntity();
		 
		 drawVBO(ent, mIndices, MISSILE);
		 
		 if (collision.detectEntityCollision(&opponents[0], ent)) {
		 //Material m = check->getMaterial();
		 //if(color == false){
		 //cout << "IN\n" << endl;
		 //check->setMaterial(Materials::wood);
		 //color = true;
		 //bigOpp.setPosition(glm::vec3(bigOpp.getPosition()[0], bigOpp.getPosition()[1] - 40.0, bigOpp.getPosition()[2]));
		 //}
		 //else if(color == true){
		 //cout << "OUT\n" << endl;
		 //check->setMaterial(Materials::jade);
		 //color = false;
		 //bigOpp.setPosition(glm::vec3(bigOpp.getPosition()[0], bigOpp.getPosition()[1] + 40.0, bigOpp.getPosition()[2]));
		 //}
		 
		 missleTime = 0;
		 beginProjectile = false;
		 delete missle;
		 }
		 }
		 */
		
		//Draw HUD
		drawHUD(fps, pitch);
		
		assert(!GLSLProgram::checkForOpenGLError(__FILE__, __LINE__));
		
		last = elapsed;
		elapsed = glfwGetTime() - start;
		
		glfwSwapBuffers(window);
		glfwPollEvents();
		frames++;
	}
	
	glDeleteFramebuffers(1, &fbo);
	glfwTerminate();
	
	return 0;
}
