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
#include <cmath>

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

// Billboard vars
GLuint CameraRight_worldspace_ID;
GLuint CameraUp_worldspace_ID;
GLuint BillboardPosID;
GLuint BillboardSizeID;
GLuint billboard_vertex_buffer;

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

float xtrans[100];
float ztrans[100];

float randNum() {
    return ((float) rand() / (RAND_MAX)) * 600.0;
}

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

	// Shader vars for billboard
	CameraRight_worldspace_ID  = glGetUniformLocation(prog, "CameraRight_worldspace");
	CameraUp_worldspace_ID  = glGetUniformLocation(prog, "CameraUp_worldspace");
	BillboardPosID = glGetUniformLocation(prog, "BillboardPos");
	BillboardSizeID = glGetUniformLocation(prog, "BillboardSize");
    
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

void drawGL(Entity *entity, int i) {
    glUseProgram(prog);
 
    int nIndices = (int)(entity->getObject(i))->shapes[0].mesh.indices.size();
    
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
    glUniform3f(lPos, 1000, 500, 1000);

    glUniform1i(renderObj, 1);

    SetMaterial(entity->getMaterial(0));
    SetModel(entity->getPosition(0), entity->getOrientation().y, entity->getScale(i));
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
    loadShapes("../Assets/models/Biplane.obj", obj[3]);
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

    // Set up characters
    vector<Entity> characters;

    float start = glfwGetTime();
    float elapsed = 0;
    float last = -1;

    Camera camera((float)glfwGetTime(), g_width, g_height);

    // Set up player
    vector<Object *> plObjs;
    vector<glm::vec3> plPoss;
    vector<glm::vec3> plScas;
    plObjs.push_back(&obj[1]);
    plPoss.push_back(camera.getPosition());
    plScas.push_back(glm::vec3(0.03,0.03,0.03));
    Entity player = Entity(plObjs, plPoss, plScas);

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
			vector<glm::vec3> scales;
			vector<Material> materials;

			objects.push_back(&obj[2]);
			objects.push_back(&obj[0]);
			positions.push_back(glm::vec3(rand() % 10 - 5, 0.3f, rand() % 10 - 5));
			positions.push_back(positions[0] + glm::vec3(0.,0.4,0.));
			scales.push_back(glm::vec3(0.3f,0.3f,0.3f));
			scales.push_back(glm::vec3(0.05f,0.05f,0.05f));
			materials.push_back(Materials::wood);
			materials.push_back(Materials::obsidian);

		    glm::vec3 cVel = glm::vec3((rand() % 2 * 2 - 1) * 0.02f, 0.0f, (rand() % 2 * 2 - 1) * 0.02f);
				        
		    Entity c(objects, positions, scales);
			c.setMaterial(0, materials[0]);
			c.setMaterial(1, materials[1]);
			c.setVelocity(cVel);
            characters.push_back(c);
            
            numObj++;
        }
        
	// Draw character
		initBillboard();
		drawBillboard(&camera);
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
	player.setPosition(0, glm::vec3(glm::inverse(view) * glm::vec4(0.0, 0.05, -1.0, 1.0)));
	player.setMaterial(0, Materials::obsidian);
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

        last = elapsed;
        elapsed = glfwGetTime() - start;
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frames++;
    }
    
    glfwTerminate();
    
    return 0;
}

