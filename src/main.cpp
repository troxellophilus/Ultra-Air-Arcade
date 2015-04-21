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

#include "Entity.hpp"
#include "Collision.hpp"
#include "Camera.hpp"
#include "Materials.hpp"
#include "Terrain.h"

#include "helper.h"
#include "GLSL.h"

//#define _DEBUG

using namespace std;
using namespace glm;

enum { TERRAIN, SKY, PLANE, NUM_VBO };

GLuint prog;
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

vector<float> terPosBuf;
vector<float> terNorBuf;
vector<unsigned int> terIndBuf;

Object obj[NUMSHAPES];
Object skydome;

Collision *sampler = new Collision();

static float g_width, g_height;

unsigned int frames = 0;
unsigned int numObj = 0;
unsigned int collisions = 0;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

static void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

/* model transforms */
void SetModel(vec3 trans, float rot, vec3 sc) {
    glm::mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
    glm::mat4 RotateY = glm::rotate( glm::mat4(1.0f), rot, glm::vec3(0.0f, 1, 0));
    glm::mat4 Sc = glm::scale(glm::mat4(1.0f), sc);
    glm::mat4 com = Trans*RotateY*Sc;
    glUniformMatrix4fv(uModelMatrix, 1, GL_FALSE, glm::value_ptr(com));
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

void initSky() {
	// glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
    //glEnable(GL_DEPTH_TEST);

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
    //glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
    //glEnable(GL_DEPTH_TEST);

    Terrain terrain = Terrain("../Assets/heightmap/Tamriel.bmp", 100.0, terPosBuf, terIndBuf, terNorBuf);

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
    
    // Set up the shader variables
    aPos = glGetAttribLocation(prog, "aPos");
    aNor = glGetAttribLocation(prog, "aNor");
    
    uViewMatrix = glGetUniformLocation(prog, "V");
    uModelMatrix = glGetUniformLocation(prog, "M");
    uProjMatrix = glGetUniformLocation(prog, "P");
    lPos = glGetUniformLocation(prog, "lPos");
    renderObj = glGetUniformLocation(prog, "renderObj");
    
    h_uMatAmb = glGetUniformLocation(prog, "UaColor");
    h_uMatDif = glGetUniformLocation(prog, "UdColor");
    h_uMatSpec = glGetUniformLocation(prog, "UsColor");
    h_uMatShine = glGetUniformLocation(prog, "Ushine");
    
    assert(glGetError() == GL_NO_ERROR);
    return true;
}

void SetMaterial(Material mat) {
    glUseProgram(prog);
    
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
    glUseProgram(prog);
 
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
    
    glUniform3f(lPos, 1000, 500, 1000);

    glUniform1i(renderObj, 0);

    SetMaterial(entity->getMaterial());
    SetModel(entity->getPosition(), entity->getOrientation().y, entity->getScale());

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
	glUseProgram(prog);
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

void drawGround() {
	glEnable(GL_CULL_FACE);
    glUseProgram(prog);
    
    glEnableVertexAttribArray(aPos);
    glBindBuffer(GL_ARRAY_BUFFER, pbo[TERRAIN]);
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLSL::enableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, nbo[TERRAIN]);
    glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[TERRAIN]);
    
    glUniform1i(renderObj, 0);
    SetMaterial(Materials::wood);
    SetModel(glm::vec3(0), 0, vec3(1));
    glDrawElements(GL_TRIANGLES, (int)terIndBuf.size(), GL_UNSIGNED_INT, 0);
    
    GLSL::disableVertexAttribArray(aPos);
    GLSL::disableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glUseProgram(0);
    assert(glGetError() == GL_NO_ERROR);
}

int main(int argc, char **argv) {
    const GLubyte* renderer;
    const GLubyte* version;
    
    srand(time(NULL));
    
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);
    
    // Initialise GLFW
    if(!glfwInit()) {
	    exit(EXIT_FAILURE);
    }
    
    //glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    g_width = 640;
    g_height = 480;
    // Open a window and create its OpenGL context
    window = glfwCreateWindow(g_width, g_height, "DTZW Code Base", NULL, NULL);
    if( window == NULL ){
        glfwTerminate();
	exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, key_callback);
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
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

    GLSL::printError(__FILE__,__LINE__);

    /* get version info */
    renderer = glGetString (GL_RENDERER);
    version = glGetString (GL_VERSION);
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported: %s\n", version);
   
    /* tell GL to only draw onto a pixel if the shape is closer to the viewer */
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);

    installShaders("shd/vert.glsl", "shd/frag.glsl");
    
    initGround();
    initSky();
    
    // Set up characters
    vector<Entity> characters;

    float start = glfwGetTime();
    float elapsed = 0;
    float last = -1;

    Camera camera((float)glfwGetTime(), g_width, g_height);

    // Set up player
    Object * plObj = &obj[3];
    glm::vec3 plPos = camera.getPosition();
    glm::vec3 plSca = glm::vec3(0.2,0.2,0.2);
    Entity player = Entity(plObj, plPos, plSca, Materials::obsidian);
    int pIndices = initVBO(&player, PLANE);

    while (!glfwWindowShouldClose(window)) {
	    float ratio;
	    int width, height;

	    glfwGetFramebufferSize(window, &width, &height);
	    glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(prog);

        // Set projection matrix
        glm::mat4 projection = camera.getProjectionMatrix();
        glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, glm::value_ptr(projection));

        // Set view matrix
        glm::mat4 view = camera.getViewMatrix();
        glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(view));

	// Create new characters if necessary
        if (int(elapsed) != int(last) && int(elapsed) % 5 == 0) {
		Object * object = &obj[3];
		glm::vec3 position = glm::vec3(rand() % 10 - 5, 0.3f, rand() % 10 - 5);
		glm::vec3 scale = glm::vec3(0.3f,0.3f,0.3f);
		Material material = Materials::emerald;

            glm::vec3 cVel = glm::vec3((rand() % 2 * 2 - 1) * 0.02f, 0.0f, (rand() % 2 * 2 - 1) * 0.02f);
            
            Entity c(object, position, scale, material);
	    c.setVelocity(cVel);
            characters.push_back(c);

            numObj++;
        }
        
	// Draw character
        for (auto &character : characters) {
            drawVBO(&character, pIndices, PLANE);
            character.update(glfwGetTime() - last);

            if (character.getPosition().x < -10 || character.getPosition().z < -10 ||
                character.getPosition().x > 10 || character.getPosition().z > 10)
                character.setVelocity(glm::vec3(0) - character.getVelocity());

            uint32_t flags = character.getFlags();
            if (sampler->sample(camera.getPosition(), &character) == Collision::status::COLLISION && !(flags & C_FLAG)) {
                numObj--;
            }
            
            for (auto &character2 : characters) {
                if (&character != &character2)
                    sampler->sampleEntitys(&character, &character2);
            }
        }

	// Draw player
	player.setPosition(glm::vec3(glm::inverse(view) * glm::vec4(0.0, 0.05, -1.0, 1.0)));
	player.setMaterial(Materials::emerald);
	player.update(glfwGetTime() - last);
	drawVBO(&player, pIndices, PLANE);
        
	// Draw terrain
        drawGround();
	drawSky();
        
	// Build keys array and update camera
	bool keys[128];
	keys['W'] = glfwGetKey(window, 'W') == GLFW_PRESS;
	keys['A'] = glfwGetKey(window, 'A') == GLFW_PRESS;
	keys['S'] = glfwGetKey(window, 'S') == GLFW_PRESS;
	keys['D'] = glfwGetKey(window, 'D') == GLFW_PRESS;
	double cx, cy;
	glfwGetCursorPos(window, &cx, &cy);
	camera.update(glfwGetTime(), keys, cx, cy);

        last = elapsed;
        elapsed = glfwGetTime() - start;
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frames++;
    }
    
    glfwTerminate();
    
    return 0;
}

