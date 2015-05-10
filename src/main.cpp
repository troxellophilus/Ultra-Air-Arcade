/* 
 * Drew Troxell & Zachary Weisman
 * Code Base
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

#include "Entity.hpp"
#include "Collision.hpp"
#include "Camera.hpp"
#include "Materials.hpp"
#include "Terrain.h"

#include "helper.h"
#include "GLSLProgram.h"

//#define _DEBUG

using namespace std;
//using namespace glm;

enum { TERRAIN, SKY, PLANE, NUM_VBO };

GLSLProgram program;

GLuint vao;
GLuint pbo[NUM_VBO];
GLuint nbo[NUM_VBO];
GLuint ibo[NUM_VBO];

GLint aPos = 0;
GLint aNor = 0;
GLuint billboard_vertex_buffer;

vector<float> terPosBuf;
vector<float> terNorBuf;
vector<unsigned int> terIndBuf;

Object obj[NUMSHAPES];
Object skydome;

Collision *sampler = new Collision();

Camera camera = Camera();
Entity player = Entity();
vector<Entity> opponents;

static float g_width, g_height;

unsigned int frames = 0;
unsigned int numObj = 0;
unsigned int collisions = 0;

float xtrans[100];
float ztrans[100];

float randNum() {
    return ((float) rand() / (RAND_MAX)) * 600.0;
}

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
    program.setUniform("M", com);
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
	//glUniform1i(renderObj, 2);
    program.setUniform("renderObj", 2);
	glm::mat4 ViewMatrix = camera->getViewMatrix();
    program.setUniform("V", ViewMatrix);
	//glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
    program.setUniform("CameraRight_worldspace", ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	//glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
    program.setUniform("CameraUp_worldspace", ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

	int i;

	for (i = 0; i < 100; i++) {
        program.setUniform("BillboardPos", 200.0f + xtrans[i], 100.0f, 200.0f + ztrans[i]);
        program.setUniform("BillboardSize", 20.0f, 20.0f);
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
	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);

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
}

void initGround() {
    Terrain terrain = Terrain("../Assets/heightmap/UltraAirArcade.bmp", 100.0, terPosBuf, terIndBuf, terNorBuf);

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

bool installShaders(const string &vShaderName, const string &fShaderName) {

    // Create Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    //new shader setup
    try
    {
        program.compileShader(vShaderName.c_str(),GLSLShader::VERTEX);
        program.compileShader(fShaderName.c_str(),GLSLShader::FRAGMENT);
        program.link();
        program.validate();
        program.use();
    }
    catch(GLSLProgramException & e) {
        cerr << e.what() << endl;
        exit( EXIT_FAILURE );
    }

    glBindFragDataLocation(program.getHandle(), 0, "outColor");

    aPos = glGetAttribLocation(program.getHandle(), "aPos");
    aNor = glGetAttribLocation(program.getHandle(), "aNor");

    return true;
}

void SetMaterial(Material mat) {
    //glUseProgram(prog);
    
    glm::vec3 amb = mat.getAmbient();
    glm::vec3 dif = mat.getDiffuse();
    glm::vec3 spc = mat.getSpecular();
    float shn = mat.getShininess();

    program.setUniform("UaColor",  amb.x, amb.y, amb.z);
    program.setUniform("UdColor", dif.x, dif.y, dif.z);
    program.setUniform("UsColor", spc.x, spc.y, spc.z);
    program.setUniform("Ushine", shn);
}

void drawVBO(Entity *entity, int nIndices, int whichbo) {
    //glUseProgram(prog);
 
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
    
    program.setUniform("lPos",1000,500,1000);

    program.setUniform("renderObj", 0);

    SetMaterial(entity->getMaterial());
    glm::mat4 Trans = glm::translate( glm::mat4(1.0f), entity->getPosition());
    glm::mat4 Orient = entity->getRotationM();
    glm::mat4 Sc = glm::scale(glm::mat4(1.0f), entity->getScale());
    glm::mat4 com = Trans*Orient*Sc;

    program.setUniform("M", com);

    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    
    // Disable and unbind
    program.disableVertexAttribArray(aPos);
    program.disableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    
}

void drawSky() {
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
    program.setUniform("M", com);
    program.setUniform("renderObj", 1);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	// Disable and unbind
	program.disableVertexAttribArray(aPos);
	program.disableVertexAttribArray(aNor);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void drawGround() {
    glEnable(GL_CULL_FACE);
    
    glEnableVertexAttribArray(aPos);
    glBindBuffer(GL_ARRAY_BUFFER, pbo[TERRAIN]);
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);

    program.enableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, nbo[TERRAIN]);
    glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[TERRAIN]);
    
    program.setUniform("renderObj", 0);
    SetMaterial(Materials::wood);
    SetModel(glm::vec3(0), glm::vec3(0,1,0), vec3(1));
    glDrawElements(GL_TRIANGLES, (int)terIndBuf.size(), GL_UNSIGNED_INT, 0);
    
    program.disableVertexAttribArray(aPos);
    program.disableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    
}

int main(int argc, char **argv) {
    const GLubyte* renderer;
    const GLubyte* version;
    
    srand(time(NULL));
    
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
    
    //glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    if(argc == 3)
    {
        g_width = atoi(argv[1]);
        g_height = atoi(argv[2]);
    }
    else
    {
        g_width = 640;
        g_height = 480;
    }

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(g_width, g_height, "Ultra Air Arcade | alpha build", NULL, NULL);
    if( window == NULL ){
        glfwTerminate();
	exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Set key and cursor callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load 3D models
    loadShapes("../Assets/models/sphere.obj", obj[0]);
    loadShapes("../Assets/models/cube.obj", obj[1]);
    loadShapes("../Assets/models/Pyro.obj", obj[2]);
    loadShapes("../Assets/models/Plane1.obj", obj[3]);
    loadShapes("../Assets/models/skydome.obj", skydome);
    std::cout << " loaded the objects " << endl;

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if ( glewInit() != GLEW_OK ) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Print opengl version & GPU info
    renderer = glGetString (GL_RENDERER);
    version = glGetString (GL_VERSION);
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported: %s\n", version);
   
    /* tell GL to only draw onto a pixel if the shape is closer to the viewer */
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);

    installShaders("shd/basic.vert", "shd/basic.frag");
    
    initSky();
    initGround();
    
    // Initialize player
    player.setObject(&obj[3]);
    player.setPosition(camera.getPosition() + glm::vec3(0.0f,1.0f,0.0f));
    player.setScale(glm::vec3(0.2,0.2,0.2));
    player.setMaterial(Materials::emerald);
    int pIndices = initVBO(&player, PLANE);

    // Initialize camera
    camera.setMode(TPC);
    camera.setPosition(glm::vec3(0,0,-10));
    camera.setClipping(0.1, 1500);
    camera.setFOV(90);
    camera.setPlayer(&player);

    // Initialize opponents
    int odx = 0;
    while (odx < 5) {
        Entity opp = Entity();
	opp.setObject(&obj[3]);
	opp.setPosition(player.getPosition() + odx * 5.f);
	opp.setScale(glm::vec3(0.2,0.2,0.2));
	opponents.push_back(opp);
	odx++;
    }

    float start = glfwGetTime();
    float elapsed = 0;
    float last = -1;

    // Frame loop
    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;

        //glfwGetFramebufferSize(window, &width, &height);
        //glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	   //glUseProgram(prog);

        // Set projection matrix
        glm::mat4 projection = camera.getProjectionMatrix();
        program.setUniform("P", camera.getProjectionMatrix());

        // Set view matrix
        glm::mat4 view = camera.getViewMatrix();
        program.setUniform("V", view);

        initBillboard();
        drawBillboard(&camera);

	// Update & draw player
	player.update();
	drawVBO(&player, pIndices, PLANE);

	// Update & draw opponents
        for (auto &opponent : opponents) {
            opponent.update();
            drawVBO(&opponent, pIndices, PLANE);
        }
        
	// Draw environment
 	drawGround();
	drawSky();
        
	// Update camera
	camera.update();

        last = elapsed;
        elapsed = glfwGetTime() - start;
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frames++;
    }
    program.printActiveUniforms();
    program.printActiveAttribs();
    glfwTerminate();
    
    return 0;
}

