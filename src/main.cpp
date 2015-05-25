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
 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <eigen3/Eigen/Dense>

#include "Entity.hpp"
#include "Collision.hpp"
#include "Camera.hpp"
#include "Materials.hpp"
#include "Terrain.h"
#include "Rules.hpp"
#include "Projectile.hpp"
#include "Skybox.hpp"

#include "helper.h"
#include "GLSL.h"
#include "GLSLProgram.h"

//#define DEBUG

using namespace std;
//using namespace glm;

enum { TERRAIN, SKY, PLANE, MISSLE, NUM_VBO };

// Program IDs
GLuint passThroughShaders;
GLuint depthCalcShaders;
GLuint renderSceneShaders;
GLuint skyBoxShaders;

GLuint vao;
GLuint pbo[NUM_VBO];
GLuint nbo[NUM_VBO];
GLuint ibo[NUM_VBO];

GLint aPos = 0;
GLint aNor = 0;
GLint lPos = 0;
GLint uViewMatrix = 0;
GLint uModelMatrix = 0;
GLint uProjMatrix = 0;
GLint renderObj = 0;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;

// Billboard variables
GLuint CameraRight_worldspace_ID;
GLuint CameraUp_worldspace_ID;
GLuint BillboardPosID;
GLuint BillboardSizeID;
GLuint billboard_vertex_buffer;

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

// For renderscene and shadowmap shaders
GLuint shadowPos;
GLuint shadowMapPos;
GLuint shadowNor;
GLuint shadowLPos;
GLuint shadowViewMatrix;
GLuint shadowModelMatrix;
GLuint shadowProjMatrix;
// End

vector<float> terPosBuf;
vector<float> terNorBuf;
vector<unsigned int> terIndBuf;

Object obj[NUMSHAPES];
Object skydome;

Collision collision = Collision();
Terrain *terrain;
Skybox *skybox;

Camera camera = Camera();
Entity player = Entity();
vector<Entity> opponents;

static float g_width, g_height;

unsigned int frames = 0;
unsigned int numObj = 0;
unsigned int collisions = 0;
unsigned int pIndices = 0;
unsigned int whichShader = 0;

float xtrans[100];
float ztrans[100];
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

float randNum() {
    return ((float) rand() / (RAND_MAX)) * 600.0;
}

int initVBO(Entity *e, int i);

// EVENT CALLBACKS
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Check escape key
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Check movement keys
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        if (key == GLFW_KEY_W) {
            camera.move(FORWARD);
	    player.throttleUp();
        }
        if (key == GLFW_KEY_S) {
            camera.move(BACK);
	    player.throttleDown();
        }
        if (key == GLFW_KEY_A) {
            camera.move(LEFT);
	    player.rollLeft();
        }
        if (key == GLFW_KEY_D) {
            camera.move(RIGHT);
	    player.rollRight();
        }
        if (mods == GLFW_MOD_SHIFT) {
            camera.move(DOWN);
        }
        if (key == GLFW_KEY_SPACE) {
            camera.move(UP);
    	}
	if (key == GLFW_KEY_M) {
            if (camera.getMode() == TPC)
	        camera.setMode(FREE);
	    else
	        camera.setMode(TPC);
	}
	if (key == GLFW_KEY_P) {
            if (player.getObject() == &obj[3])
                player.setObject(&obj[2]);
	    else
		player.setObject(&obj[3]);
	    
	    pIndices = initVBO(&player, PLANE);
	}
    if(key == GLFW_KEY_X){
           if(!beginProjectile){
              beginProjectile = true;
              missle = new Projectile(projectileEntity, true, player.getPosition(), opponents[1].getPosition());
              missleTime = elapsed;
              //cout << "MISSLE TIME: " << missleTime << endl;
              //cout << "New Projectile" << endl;
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
    glm::mat4 com = Trans*Orient*Sc;
    glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(com));
}

glm::mat4 SetModel(vec3 trans, glm::vec3 rot, vec3 sc, int num) {
    glm::mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
    glm::mat4 Orient = glm::orientation(rot, glm::vec3(0, 1, 0));
    glm::mat4 Sc = glm::scale(glm::mat4(1.0f), sc);
    glm::mat4 com = Trans*Orient*Sc;
    return com;
}

int initVBO(Entity *e, int whichbo) {
    vector<float> posBuf, norBuf;
    vector<unsigned int> indBuf;

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

    return (int)indBuf.size();
}

void initBillboard() {
// Set up billboard
	static const GLfloat g_vertex_buffer_data[] = { 
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f, 0.5f, 0.0f,
		  0.5f, 0.5f, 0.0f,
	};

	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);
}

void drawBillboard(Camera *camera) {
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform1i(renderObj, 2);
	glm::mat4 ViewMatrix = camera->getViewMatrix();
	glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

	int i;

	for (i = 0; i < 100; i++) {
		glUniform3f(BillboardPosID, 200.0f + xtrans[i], 100.0f, 200.0f + ztrans[i]); // The billboard will be just above the cube
		glUniform2f(BillboardSizeID, 20.0f, 20.0f);     // and 1m*12cm, because it matches its 256*32 resolution =)

		// Enable and bind position array for drawing
		glEnableVertexAttribArray(aPos);
		glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
		glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		// Draw the billboard !
		// This draws a triangle_strip which looks like a quad.
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glDisableVertexAttribArray(0);
}

void initSky() {
    const vector<float> &posBuf = skydome.shapes[0].mesh.positions;
    glGenBuffers(1, &pbo[SKY]);
    glBindBuffer(GL_ARRAY_BUFFER, pbo[SKY]);
    glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
    
    // Send the normal array to the GPU
    const vector<float> &norBuf = skydome.shapes[0].mesh.normals;
    glGenBuffers(1, &nbo[SKY]);
    glBindBuffer(GL_ARRAY_BUFFER, nbo[SKY]);
    glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
    
    // Send the index array to the GPU
    const vector<unsigned int> &indBuf = skydome.shapes[0].mesh.indices;
    glGenBuffers(1, &ibo[SKY]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[SKY]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
    
    // Unbind the arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GLSL::checkVersion();
    assert(glGetError() == GL_NO_ERROR);
}

void initGround() {

    terrain = (Terrain *)malloc(sizeof(Terrain));
    *terrain = Terrain("../Assets/heightmap/UltraAirArcade.bmp", 100.0, terPosBuf, terIndBuf, terNorBuf);

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
    GLSL::checkVersion();
    assert(glGetError() == GL_NO_ERROR);
}

void initShaderVars() {
	// Set up the shader variables
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

    // Shader vars for billboard
    CameraRight_worldspace_ID  = glGetUniformLocation(passThroughShaders, "CameraRight_worldspace");
    CameraUp_worldspace_ID  = glGetUniformLocation(passThroughShaders, "CameraUp_worldspace");
    BillboardPosID = glGetUniformLocation(passThroughShaders, "BillboardPos");
    BillboardSizeID = glGetUniformLocation(passThroughShaders, "BillboardSize");
    
    depthBiasID = 	glGetUniformLocation(renderSceneShaders, "depthBiasMVP");
	shadowMapID = 	glGetUniformLocation(renderSceneShaders, "shadowMap");
	shadowViewMatrix = glGetUniformLocation(renderSceneShaders, "V");
    shadowModelMatrix = glGetUniformLocation(renderSceneShaders, "M");
    shadowProjMatrix = glGetUniformLocation(renderSceneShaders, "P");
	shadowLPos = glGetUniformLocation(renderSceneShaders, "lPos");
	
	depthMatrixID = glGetUniformLocation(depthCalcShaders, "depthMVP");
	
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
	assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
}

GLuint installShaders(const string &vShaderName, const string &fShaderName) {
	GLuint prog;
    GLint rc;
    GLSL::printError(__FILE__,__LINE__);

    // Create Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // Create shader handles
    GLuint VS = glCreateShader(GL_VERTEX_SHADER);
    GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
    GLSL::printError(__FILE__,__LINE__);
    
    // Read shader sources
    const char *vshader = GLSL::textFileRead(vShaderName.c_str());
    const char *fshader = GLSL::textFileRead(fShaderName.c_str());
    glShaderSource(VS, 1, &vshader, NULL);
    glShaderSource(FS, 1, &fshader, NULL);
    GLSL::printError(__FILE__,__LINE__);
    
    // Compile vertex shader
    glCompileShader(VS);
    GLSL::printError(__FILE__,__LINE__);
    glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
    GLSL::printShaderInfoLog(VS);
    if(!rc) {
        printf("Error compiling vertex shader %s\n", vShaderName.c_str());
        return false;
    }

    // Compile fragment shader
    glCompileShader(FS);
    GLSL::printError(__FILE__,__LINE__);
    glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
    GLSL::printShaderInfoLog(FS);
    if(!rc) {
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
    GLSL::printError(__FILE__,__LINE__);
    glGetProgramiv(prog, GL_LINK_STATUS, &rc);
    GLSL::printProgramInfoLog(prog);
    if(!rc) {
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

void drawVBO(Entity *entity, int nIndices, int whichbo) {
    glUseProgram(passThroughShaders);
 
    // Enable and bind position array for drawing
    glEnableVertexAttribArray(aPos);
    glBindBuffer(GL_ARRAY_BUFFER, pbo[whichbo]);
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Enable and bind normal array for drawing
    glEnableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, nbo[whichbo]);
    glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Bind index array for drawing
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[whichbo]);
    
    glUniform3f(lPos, 200, 1000, 200);

    glUniform1i(renderObj, 0);

    SetMaterial(entity->getMaterial());
    glm::mat4 Trans = glm::translate( glm::mat4(1.0f), entity->getPosition());
    glm::mat4 Orient = entity->getRotationM();
    glm::mat4 Sc = glm::scale(glm::mat4(1.0f), entity->getScale());
    glm::mat4 com = Trans*Orient*Sc;
    glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(com));

    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    
    // Disable and unbind
    GLSL::disableVertexAttribArray(aPos);
    GLSL::disableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // Last lines
    glUseProgram(0);
    assert(glGetError() == GL_NO_ERROR);
}

void drawSky() {
	glUseProgram(passThroughShaders);
	int nIndices = (int)skydome.shapes[0].mesh.indices.size();

	glEnableVertexAttribArray(aPos);
	glBindBuffer(GL_ARRAY_BUFFER, pbo[SKY]);
	glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(aNor);
	glBindBuffer(GL_ARRAY_BUFFER, nbo[SKY]);
	glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[SKY]);

	// Bind index array for drawing
	// Compute and send the projection matrix - leave this as is

	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(750.0f, 750.0f, 750.0f)); 
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(512.0f, 0.0f, 512.0f));

	glm::mat4 com = trans * scale;
	glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(com));
	glUniform1i(renderObj, 1);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	// Disable and unbind
	GLSL::disableVertexAttribArray(aPos);
	GLSL::disableVertexAttribArray(aNor);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Last lines
	glUseProgram(0);
	assert(glGetError() == GL_NO_ERROR);
}

void drawGround(glm::mat4 projMatrix, glm::mat4 viewMatrix) {
	/*glGenFramebuffers(1, &fbo);
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
	glDrawBuffer(GL_NONE);*/
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
    glUseProgram(depthCalcShaders);
	
    depthModelMatrix = SetModel(glm::vec3(0), glm::vec3(0,1,0), vec3(1), 1);
    depthViewMatrix = glm::lookAt(lightPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    
    depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
    glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0]);
    assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
    
    glEnableVertexAttribArray(shadowMapPos);
    glBindBuffer(GL_ARRAY_BUFFER, pbo[TERRAIN]);
    glVertexAttribPointer(shadowPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[TERRAIN]);
    glDrawElements(GL_TRIANGLES, (int)terIndBuf.size(), GL_UNSIGNED_INT, 0);
    GLSL::disableVertexAttribArray(shadowMapPos);
    assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, g_width, g_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(renderSceneShaders);
	glUniform3f(shadowLPos, lightPosition.x, lightPosition.y, lightPosition.z);
	assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
	
	glm::mat4 projection = projMatrix;
    glUniformMatrix4fv(shadowProjMatrix, 1, GL_FALSE, glm::value_ptr(projection));
	assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));

    // Set view matrix
    glm::mat4 view = viewMatrix;
    glUniformMatrix4fv(shadowViewMatrix, 1, GL_FALSE, glm::value_ptr(view));
    assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
    
    glm::mat4 model = SetModel(glm::vec3(0), glm::vec3(0,1,0), vec3(1), 1);
    glUniformMatrix4fv(shadowModelMatrix, 1, GL_FALSE, glm::value_ptr(model));
    assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
    
    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glUniform1i(shadowMapID, 0);
	assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
	
	glEnableVertexAttribArray(shadowPos);
    glBindBuffer(GL_ARRAY_BUFFER, pbo[TERRAIN]);
    glVertexAttribPointer(shadowPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));

    GLSL::enableVertexAttribArray(shadowNor);
    glBindBuffer(GL_ARRAY_BUFFER, nbo[TERRAIN]);
    glVertexAttribPointer(shadowNor, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[TERRAIN]);
    
    glDrawElements(GL_TRIANGLES, (int)terIndBuf.size(), GL_UNSIGNED_INT, 0);
    GLSL::disableVertexAttribArray(shadowPos);
    GLSL::disableVertexAttribArray(shadowNor);
    //glDeleteFramebuffers(1, &fbo);
    assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
	// End shadow rendering section
}

void checkPlayerCollisions() {
    if (collision.detectTerrainCollision(player, terrain)) {
        
        if (!collisionDetectedTerrain) {
            collisionCount++;
            collisionDetectedTerrain = !collisionDetectedTerrain;
            //printf("Detected player collision with terrain: %d\n", collisionCount);

            glm::vec3 playerPos = player.getPosition();
            Eigen::Vector3f convertedPos = Eigen::Vector3f(playerPos.x, playerPos.y, playerPos.z);
            Eigen::Vector3f normalVec = terrain->getNormal(convertedPos);
            glm::vec3 convertedNor = glm::vec3(normalVec(0), normalVec(1), normalVec(2));
            player.setPosition(playerPos + (convertedNor * 3.f));
            player.setThrust(0.f);
            player.setVelocity(glm::vec3(0.f, 0.f, 0.f));
        }
    }
    else {
        collisionDetectedTerrain =  false;
    }
}

void checkOpponentCollisions(Entity &opponent) {
    if (collision.detectEntityCollision(player, opponent)) {
        if (!collisionDetectedOpponent) {
            collisionCount++;
            collisionDetectedOpponent = !collisionDetectedOpponent;
            printf("Detected collision with enemy: %d\n", collisionCount);

            player.setVelocity(player.getVelocity() * -0.25f);
        }
    }
    else {
        collisionDetectedOpponent = !collisionDetectedOpponent;
    }

    if (collision.detectTerrainCollision(opponent, terrain)) {
        //collisionCount++;
        //printf("Detected opponent collision with terrain: %d\n", collisionCount);
    }
}

int main(int argc, char **argv) {
    const GLubyte* renderer;
    const GLubyte* version;
    
    srand(time(NULL));
    
    if (argc > 1) {
    	argv++;
    	if (**argv == '1') renderShadows = true;
    	else renderShadows = false;
    } else renderShadows = false;
    
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    int i;
    for (i = 0; i < 100; i++) {
        xtrans[i] = randNum();
        ztrans[i] = randNum();
    }    

    // Initialise GLFW
    if(!glfwInit()) {
	    exit(EXIT_FAILURE);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    g_width = 640;
    g_height = 480;

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(g_width, g_height, "Ultra Air Arcade | alpha build", NULL, NULL);
    if( window == NULL ){
        glfwTerminate();
		exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    GLSLProgram::checkForOpenGLError(__FILE__,__LINE__);
    // Set key and cursor callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // Load 3D models
    loadShapes("../Assets/models/sphere.obj", obj[0]);
    loadShapes("../Assets/models/cube.obj", obj[1]);
    loadShapes("../Assets/models/Pyro.obj", obj[2]);
    loadShapes("../Assets/models/Plane1.obj", obj[3]);
    loadShapes("../Assets/models/missile.obj", obj[4]);
    loadShapes("../Assets/models/skydome.obj", skydome);
    std::cout << " loaded the objects " << endl;

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if ( glewInit() != GLEW_OK ) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    GLSLProgram::checkForOpenGLError(__FILE__,__LINE__);
    // Print opengl version & GPU info
    renderer = glGetString (GL_RENDERER);
    version = glGetString (GL_VERSION);
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported: %s\n", version);
   
   	assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
    /* tell GL to only draw onto a pixel if the shape is closer to the viewer */
    glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
    
    depthCalcShaders = installShaders("shd/depthmap_vert.glsl", "shd/depthmap_frag.glsl");
    whichShader = 1;
    renderSceneShaders = installShaders("shd/renderscene_vert.glsl", "shd/renderscene_frag.glsl");
    passThroughShaders = installShaders("shd/basic.vert", "shd/basic.frag");
    skyBoxShaders = installShaders("shd/skybox_vert.glsl", "shd/skybox_frag.glsl");
	initShaderVars();    

    //initSky();
    //initGround();
    initBillboard();
	skybox = new Skybox(skyBoxShaders);
	skybox->initShaderVars();
    
    // Initialize player
    player.setObject(&obj[3]);
    player.setPosition(glm::vec3(200.0f,35.f,200.0f));
    player.setScale(glm::vec3(0.2,0.2,0.2));
    player.setMaterial(Materials::emerald);
    player.calculateBoundingSphereRadius();
    pIndices = initVBO(&player, PLANE);

    // Initialize camera
    camera.setMode(TPC);
    camera.setPosition(glm::vec3(0,0,-10));
    camera.setClipping(0.1, 1500);
    camera.setFOV(90);
    camera.setPlayer(&player);

    //cout << "Init\n";
    projectileEntity.setObject(&obj[4]);
    projectileEntity.setPosition(glm::vec3(player.getPosition()[0], player.getPosition()[1], player.getPosition()[2]));
    projectileEntity.setScale(glm::vec3(2.0, 2.0, 2.0));
    projectileEntity.setMaterial(Materials::emerald);
    int mIndices = initVBO(&projectileEntity, MISSLE);
    projectileEntity.calculateBoundingSphereRadius();

    // Initialize opponents
    int odx = 1;
    while (odx < 6) {
        Entity opp = Entity();
		opp.setObject(&obj[3]);
		glm::vec3 epos = player.getPosition();
		opp.setPosition(glm::vec3(epos.x + odx * 0.7f, epos.y, epos.z + odx * 0.4f));
		opp.setScale(glm::vec3(0.2,0.2,0.2));
		    opp.calculateBoundingSphereRadius();
		opponents.push_back(opp);
		odx++;
    }

    // Initialize game rules
    Rules rules = Rules();
    rules.setAgents(&opponents);

    Entity bigOpp = Entity();
    bigOpp.setObject(&obj[3]);
    bigOpp.setPosition(player.getPosition() + odx * 5.f);
    bigOpp.setMaterial(Materials::jade);
    bigOpp.setScale(glm::vec3(20.0,20.0,20.0));
    bigOpp.calculateBoundingSphereRadius();

    float start = glfwGetTime();
    elapsed = 0;
    float last = -1;

    Projectile pathPlane = Projectile(bigOpp, true, bigOpp.getPosition(), bigOpp.getPosition());
    bool color = false;
    // Frame loop
    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;

        assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
        
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(passThroughShaders);

        // Set projection matrix
        glm::mat4 projection = camera.getProjectionMatrix();
        glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, glm::value_ptr(projection));

        // Set view matrix
        glm::mat4 view = camera.getViewMatrix();
        glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(view));
        
        drawBillboard(&camera);
        assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));

		// Update the rules and game state
		rules.update();

		// Draw environment
		//drawGround(projection, view);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
		glUseProgram(passThroughShaders);
		//drawSky();
		//skybox->render(view, projection);
		assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
		fprintf(stderr, "11\n");
		// Update & draw player
		player.update();
		assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
		drawVBO(&player, pIndices, PLANE);
		checkPlayerCollisions();
		assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
		fprintf(stderr, "10\n");
		// Update & draw opponents
		for (auto &opponent : opponents) {
		    opponent.update();
		    drawVBO(&opponent, pIndices, PLANE);
		    checkOpponentCollisions(opponent);
		    assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
		}

		// Update camera
		camera.update();
		assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));

		pathPlane.runProjectile(true, elapsed, bigOpp.getPosition(), bigOpp.getPosition());
		Entity *check = pathPlane.getEntity();

		drawVBO(check, pIndices, PLANE);
/*
		if(beginProjectile == true){
			//cout << "MISSLE TIME: " << missleTime << endl;
			//cout << "ELAPSED: " << elapsed << endl;

			int isDone = missle->runProjectile(false, elapsed - missleTime,
			player.getPosition(), check->getPosition());

			Entity* ent = missle->getEntity();

			drawVBO(ent, mIndices, MISSLE);

			if(collision.detectEntityCollision(*check, *ent)) {
				//Material m = check->getMaterial();
				if(color == false){
					//cout << "IN\n" << endl;
					check->setMaterial(Materials::wood);
					color = true;
					bigOpp.setPosition(glm::vec3(bigOpp.getPosition()[0], bigOpp.getPosition()[1] - 40.0, bigOpp.getPosition()[2]));
				} else if(color == true){
				   //cout << "OUT\n" << endl;
				   check->setMaterial(Materials::jade);
				   color = false;
				   bigOpp.setPosition(glm::vec3(bigOpp.getPosition()[0], bigOpp.getPosition()[1] + 40.0, bigOpp.getPosition()[2]));
				}

				missleTime = 0;
				beginProjectile = false;
				delete missle;
			}
		}*/

     	assert(!GLSLProgram::checkForOpenGLError(__FILE__,__LINE__));
		// Print DEBUG messages
		/*if (argc > 1 && argv[1][0] == 'd' && frames % 5 == 0) {
			printf("Player Pos: %f, %f, %f\n", player.getPosition().x, player.getPosition().y, player.getPosition().z);
		}*/

        last = elapsed;
        elapsed = glfwGetTime() - start;
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frames++;
    }
    
    
    glDeleteFramebuffers(1, &fbo);
    glDeleteProgram(renderSceneShaders);
    glDeleteProgram(depthCalcShaders);
    glDeleteProgram(passThroughShaders);
    glfwTerminate();
    
    return 0;
}

