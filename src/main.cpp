/* 
 * Drew Troxell & Zachary Weisman
 * Code Base
 * CPE 476
 */

#include <GL/glew.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

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


GLuint prog;
GLuint vao;
GLuint posBufObj = 0;
GLuint norBufObj = 0;
GLuint indBufObj = 0;

GLuint posBufObjG = 0;
GLuint norBufObjG = 0;
GLuint indBufObjG = 0;

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

void initGL(Object *o, int i) {
    glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    // Send the position array to the GPU
    const vector<float> &posBuf = o->shapes[0].mesh.positions;
    glGenBuffers(1, &posBufObj);
    glBindBuffer(GL_ARRAY_BUFFER, posBufObj);
    glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
    
    // Send the normal array to the GPU
    const vector<float> &norBuf = o->shapes[0].mesh.normals;
    glGenBuffers(1, &norBufObj);
    glBindBuffer(GL_ARRAY_BUFFER, norBufObj);
    glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
    
    // Send the index array to the GPU
    const vector<unsigned int> &indBuf = o->shapes[0].mesh.indices;
    glGenBuffers(1, &indBufObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
    
    // Unbind the arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GLSL::checkVersion();
    assert(glGetError() == GL_NO_ERROR);
}

void initSky() {
	 glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    const vector<float> &posBuf = skydome.shapes[0].mesh.positions;
    glGenBuffers(1, &posBufObj);
    glBindBuffer(GL_ARRAY_BUFFER, posBufObj);
    glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
    
    // Send the normal array to the GPU
    const vector<float> &norBuf = skydome.shapes[0].mesh.normals;
    glGenBuffers(1, &norBufObj);
    glBindBuffer(GL_ARRAY_BUFFER, norBufObj);
    glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
    
    // Send the index array to the GPU
    const vector<unsigned int> &indBuf = skydome.shapes[0].mesh.indices;
    glGenBuffers(1, &indBufObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObj);
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

    glGenBuffers(1, &posBufObjG);
    glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
    glBufferData(GL_ARRAY_BUFFER, terPosBuf.size()*sizeof(float), &terPosBuf[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &norBufObjG);
    glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
    glBufferData(GL_ARRAY_BUFFER, terNorBuf.size()*sizeof(float), &terNorBuf[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &indBufObjG);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjG);
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

void drawGL(Entity *character, int i) {
    glUseProgram(prog);
 
    int nIndices;
    if (i == 0) {
        nIndices = (int)(character->getObject(0))->shapes[0].mesh.indices.size();
    }
    else {
        nIndices = (int)(character->getObject(1))->shapes[0].mesh.indices.size();
    }
    
    // Enable and bind position array for drawing
    glEnableVertexAttribArray(aPos);
    glBindBuffer(GL_ARRAY_BUFFER, posBufObj);
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Enable and bind normal array for drawing
    glEnableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, norBufObj);
    glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObj);
    
    // Bind index array for drawing
    // Compute and send the projection matrix - leave this as is
    glUniform3f(lPos, 10, 10, 10);
    if (i == 0) {
        glUniform1i(renderObj, 0);
        SetMaterial(character->getMaterial(0));
        SetModel(character->getPosition(0), character->getOrientation().y, vec3(0.3f, 0.3f, 0.3f));
        glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    }
    else {
        glUniform1i(renderObj, 0);
        SetMaterial(character->getMaterial(1));
        SetModel(character->getPosition(1), 0, vec3(0.05, 0.05, 0.05));
        glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    }
    
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
	glBindBuffer(GL_ARRAY_BUFFER, posBufObj);
	glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(aNor);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObj);
	glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObj);

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
    glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLSL::enableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
    glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjG);
    
    glUniform1i(renderObj, 0);
    SetMaterial(Materials::jade);
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
    
    initGround();

    installShaders("shd/vert.glsl", "shd/frag.glsl");
    
    // Set up characters
    vector<Entity> characters;

    float start = glfwGetTime();
    float elapsed = 0;
    float last = -1;

    Camera camera((float)glfwGetTime(), g_width, g_height);

    // Set up player
    vector<Object *> plObjs;
    vector<glm::vec3> plPoss;
    plObjs.push_back(&obj[1]);
    plPoss.push_back(camera.getPosition());
    Entity player = Entity(plObjs, plPoss);

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
		vector<Object *> objects;
		vector<glm::vec3> positions;
		vector<Material> materials;

		objects.push_back(&obj[2]);
		objects.push_back(&obj[0]);
		positions.push_back(glm::vec3(rand() % 10 - 5, 0.3f, rand() % 10 - 5));
		positions.push_back(positions[0] + glm::vec3(0.,0.4,0.));
		materials.push_back(Materials::wood);
		materials.push_back(Materials::obsidian);

            glm::vec3 cVel = glm::vec3((rand() % 2 * 2 - 1) * 0.02f, 0.0f, (rand() % 2 * 2 - 1) * 0.02f);
            
            Entity c(objects, positions);
	    c.setMaterial(0, materials[0]);
	    c.setMaterial(1, materials[1]);
	    c.setVelocity(cVel);
            characters.push_back(c);
            
            numObj++;
        }
        
	// Draw character indicators
        initGL(characters.begin()->getObject(1), 0);
        for (auto &character : characters) {
            drawGL(&character, 1);
        }
        
	// Draw character
        initGL(characters.begin()->getObject(0), 0);
        for (auto &character : characters) {
            drawGL(&character, 0);
            character.update(glfwGetTime() - last);

            if (character.getPosition(0).x < -10 || character.getPosition(0).z < -10 ||
                character.getPosition(0).x > 10 || character.getPosition(0).z > 10)
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
	initGL(player.getObject(0), 0);
	player.update(glfwGetTime() - last);
	drawGL(&player, 0);
        
	// Draw terrain
   drawGround();
	initSky();
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
	player.setPosition(0, camera.getPosition());

        last = elapsed;
        elapsed = glfwGetTime() - start;
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frames++;
    }
    
    glfwTerminate();
    
    return 0;
}

