/* Zachary Weisman
 * Program 4 - Winter 2015
 * CPE 471 - 01
 */

#include "Character.hpp"
#include "Collision.hpp"

//#define _DEBUG

using namespace std;
using namespace glm;

GLFWwindow* window;

GLuint prog;
GLuint posBufObj = 0;
GLuint norBufObj = 0;
GLuint indBufObj = 0;

GLuint posBufObjG = 0;
GLuint norBufObjG = 0;

GLint aPos = 0;
GLint aNor = 0;
GLint lPos = 0;
GLint uViewMatrix = 0;
GLint uModelMatrix = 0;
GLint uProjMatrix = 0;
GLint renderObj = 0;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;

Object obj[NUMSHAPES];

Collision *sampler = new Collision();

vec3 TreeTranslations[] = {
    vec3(3, 0.7, 6),
    vec3(6, 0.7, 9),
    vec3(9, 0.7, 3),
    vec3(9, 0.7, -6),
    vec3(3, 0.7, -3),
    vec3(-3, 0.7, -3),
    vec3(-6, 0.7, -12),
    vec3(-9, 0.7, -3),
    vec3(-12, 0.7, -3),
    vec3(-6, 0.7, 6),
};

vec3 WheelTranslations[] = {
    vec3(1.85, 0.1, 1.8),
    vec3(2.25, 0.1, 2.4),
    vec3(1.85, 0.1, 2.4),
    vec3(2.25, 0.1, 1.8),
};

vec3 BodyTranslations[] = {
    vec3(2.05, 0.15, 2.05),
    vec3(2.05, 0.3, 2.05),
};

static float g_width, g_height;
float theta[NUMSHAPES], phi[NUMSHAPES], light_x[NUMSHAPES], light_z[NUMSHAPES];

glm::vec3 position = glm::vec3( 0, 0.2, 0 );
float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;
unsigned int frames = 0;
unsigned int numObj = 0;
unsigned int collisions = 0;

void SetProjectionMatrix() {
    glm::mat4 Projection = glm::perspective(70.0f, (float)g_width/g_height, 0.1f, 100.f);
    glUniformMatrix4fv(uProjMatrix, 1, GL_FALSE, glm::value_ptr(Projection));
}

void SetView() {
    static double last = glfwGetTime();
    double current = glfwGetTime(), x, y;
    float diff = (float)(current - last);
    
    glfwGetCursorPos(window, &x, &y);
    
    // Reset mouse position for next frame
    //if (!__APPLE__)
#ifndef __APPLE__
    glfwSetCursorPos(window, 1024/2, 768/2);
#endif
    
    // Compute new orientation
    horizontalAngle += SCROLLSPEED * float(1024/2 - x );
    float newAngle = SCROLLSPEED * float(768 / 2 - y);
    
    if (verticalAngle + newAngle <= (3.14f/2.0f) * 0.88 && verticalAngle + newAngle >= (-3.14f/2.0f) * 0.88)
        verticalAngle += newAngle;
    
    vec3 direction = vec3(
                          cos(verticalAngle) * sin(horizontalAngle),
                          sin(verticalAngle),
                          cos(verticalAngle) * cos(horizontalAngle)
                          );
    
    vec3 right = vec3(sin(horizontalAngle - 3.14f/2.0f),
                      0, cos(horizontalAngle - 3.14f/2.0f)
                      );
    
    // Move forward
    glm::vec3 newPosition;
    if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS) {
        newPosition = position + direction * diff * CAMSPEED;
        if (newPosition.y >= 0.05 && newPosition.y <= 1.0f)
            position = newPosition;
    }
    if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS) {
        newPosition = position - direction * diff * CAMSPEED;
        if (newPosition.y >= 0.05 && newPosition.y <= 1.0f)
            position -= direction * diff * CAMSPEED;
    }
    if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
        position += right * diff * CAMSPEED;
    if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS)
        position -= right * diff * CAMSPEED;
    
    // Camera matrix
    int32_t upAngle = 1; //!__APPLE__ ? 1 : -1;
    mat4 Camera = glm::lookAt(position, position+direction, vec3(0, upAngle, 0));
    last = current;
    
    glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(Camera));
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

void initVars() {
    for (int i = 0; i < NUMSHAPES; i++) {
        //initialize the modeltrans matrix stack
        theta[i] = 0;
        light_x[i] = 0;
        light_z[i] = 0;
    }
}

void initGround() {
    float G_edge = 20;
    GLfloat g_backgnd_data[] = {
        -G_edge, 0.0f, -G_edge,
        -G_edge,  0.0f, G_edge,
        G_edge, 0.0f, -G_edge,
        -G_edge,  0.0f, G_edge,
        G_edge, 0.0f, -G_edge,
        G_edge, 0.0f, G_edge,
    };
    
    GLfloat nor_Buf_G[] = {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };
    
    glGenBuffers(1, &posBufObjG);
    glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_backgnd_data), g_backgnd_data, GL_STATIC_DRAW);
    
    glGenBuffers(1, &norBufObjG);
    glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
    glBufferData(GL_ARRAY_BUFFER, sizeof(nor_Buf_G), nor_Buf_G, GL_STATIC_DRAW);
    
}

bool installShaders(const string &vShaderName, const string &fShaderName) {
    GLint rc;
    
    // Create shader handles
    GLuint VS = glCreateShader(GL_VERTEX_SHADER);
    GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read shader sources
    const char *vshader = GLSL::textFileRead(vShaderName.c_str());
    const char *fshader = GLSL::textFileRead(fShaderName.c_str());
    glShaderSource(VS, 1, &vshader, NULL);
    glShaderSource(FS, 1, &fshader, NULL);
    
    // Compile vertex shader
    glCompileShader(VS);
    GLSL::printError();
    glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
    GLSL::printShaderInfoLog(VS);
    if(!rc) {
        printf("Error compiling vertex shader %s\n", vShaderName.c_str());
        return false;
    }
    
    // Compile fragment shader
    glCompileShader(FS);
    GLSL::printError();
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
    glLinkProgram(prog);
    GLSL::printError();
    glGetProgramiv(prog, GL_LINK_STATUS, &rc);
    GLSL::printProgramInfoLog(prog);
    if(!rc) {
        printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
        return false;
    }
    
    // Set up the shader variables
    aPos = GLSL::getAttribLocation(prog, "aPos");
    aNor = GLSL::getAttribLocation(prog, "aNor");
    
    uViewMatrix = GLSL::getUniformLocation(prog, "V");
    uModelMatrix = GLSL::getUniformLocation(prog, "M");
    uProjMatrix = GLSL::getUniformLocation(prog, "P");
    lPos = GLSL::getUniformLocation(prog, "lPos");
    renderObj = GLSL::getUniformLocation(prog, "renderObj");
    
    h_uMatAmb = GLSL::getUniformLocation(prog, "UaColor");
    h_uMatDif = GLSL::getUniformLocation(prog, "UdColor");
    h_uMatSpec = GLSL::getUniformLocation(prog, "UsColor");
    h_uMatShine = GLSL::getUniformLocation(prog, "Ushine");
    
    assert(glGetError() == GL_NO_ERROR);
    return true;
}

void SetMaterial(int i) {
    glUseProgram(prog);
    switch (i) {
        case 0: // Emerald
            glUniform3f(h_uMatAmb, 0.0215, 0.2745, 0.0215);
            glUniform3f(h_uMatDif, 0.07568, 0.61424, 0.7568);
            glUniform3f(h_uMatSpec, 0.633, 0.727811, 0.633);
            glUniform1f(h_uMatShine, 150.8);
            break;
        case 1: // Jade
            glUniform3f(h_uMatAmb, 0.135, 0.2225, 0.1575);
            glUniform3f(h_uMatDif, 0.54, 0.89, 0.63);
            glUniform3f(h_uMatSpec, 0.316228, 0.316228, 0.316228);
            glUniform1f(h_uMatShine, 100.8);
            break;
        case 2: // Wood
            glUniform3f(h_uMatAmb, 0.3, 0.2, 0.05);
            glUniform3f(h_uMatDif, 0.6, 0.41, 0.29);
            glUniform3f(h_uMatSpec, 0.0, 0.0, 0.0);
            glUniform1f(h_uMatShine, 3.0);
            break;
        case 3: // Obsidian
            glUniform3f(h_uMatAmb, 0.05375, 0.05, 0.06626);
            glUniform3f(h_uMatDif, 0.18275, 0.17, 0.22525);
            glUniform3f(h_uMatSpec, 0.332741, 0.328634, 0.346435);
            glUniform1f(h_uMatShine, 38.4);
            break;
        case 4: // Green Plastic
            glUniform3f(h_uMatAmb, 0.15, 0.4, 0.15);
            glUniform3f(h_uMatDif, 0.1, 0.35, 0.1);
            glUniform3f(h_uMatSpec, 0.45, 0.55, 0.45);
            glUniform1f(h_uMatShine, 100.0);
            break;
    }
}

void drawGL(Character *character, int i) {
    glUseProgram(prog);
    
    SetProjectionMatrix();
    SetView();
    
    int nIndices;
    if (i == 0) {
        nIndices = (int)(character->getObject())->shapes[0].mesh.indices.size();
    }
    else {
        nIndices = (int)(character->getIndicator())->shapes[0].mesh.indices.size();
    }
    
    // Enable and bind position array for drawing
    GLSL::enableVertexAttribArray(aPos);
    glBindBuffer(GL_ARRAY_BUFFER, posBufObj);
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Enable and bind normal array for drawing
    GLSL::enableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, norBufObj);
    glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObj);
    
    // Bind index array for drawing
    // Compute and send the projection matrix - leave this as is
    glUniform3f(lPos, 10, 10, 10);
    if (i == 0) {
        glUniform1i(renderObj, 1);
        SetMaterial(character->getMaterial());
        SetModel(character->getPosition(), character->getOrientation().y, vec3(0.3f, 0.3f, 0.3f));
        glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    }
    else {
        glUniform1i(renderObj, 1);
        SetMaterial(character->getIMaterial());
        SetModel(character->getIPosition(), 0, vec3(0.05, 0.05, 0.05));
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

void drawGround() {
    glUseProgram(prog);
    
    SetModel(vec3(0), 0, vec3(1));
    glUniform1i(renderObj, 0);
    
    glEnableVertexAttribArray(aPos);
    glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    GLSL::enableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
    glVertexAttribPointer(aNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    GLSL::disableVertexAttribArray(aPos);
    GLSL::disableVertexAttribArray(aNor);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glUseProgram(0);
    assert(glGetError() == GL_NO_ERROR);
}

void reshapeGL(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    g_width = w;
    g_height = h;
}

int main(int argc, char **argv) {
    srand(time(NULL));
    
    // Initialise GLFW
    if( !glfwInit() ) {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    
    g_width = 1024;
    g_height = 768;
    // Open a window and create its OpenGL context
    window = glfwCreateWindow( g_width, g_height, "Lab 8/Program 4", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    loadShapes("sphere.obj", obj[0]);
    loadShapes("cube.obj", obj[1]);
    loadShapes("Pyro.obj", obj[2]);
    
    std::cout << " loaded the objects " << endl;
    // Initialize GLEW
    if ( glewInit() != GLEW_OK ) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    installShaders("vert.glsl", "frag.glsl");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    initVars();
    initGround();
    
    // Set up characters
    vector<Character> characters;
    initText2D( "Holstein.DDS" );
    
    float start = glfwGetTime();
    float elapsed = 0;
    float last = -1;
    
    do {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (int(elapsed) != int(last) && int(elapsed) % 5 == 0) {
            glm::vec3 cPos = glm::vec3(rand() % 10 - 5, 0.3f, rand() % 10 - 5);
            glm::vec3 cVel = glm::vec3((rand() % 2 * 2 - 1) * 0.02f, 0.0f, (rand() % 2 * 2 - 1) * 0.02f);
            uint32_t cMat = 2;
            uint32_t iMat = 3;
            
            Character c(&obj[2], &obj[0], cPos, cVel, cMat, iMat);
            characters.push_back(c);
            
            numObj++;
        }
        
        initGL(characters.begin()->getIndicator(), 0);
        for (auto &character : characters) {
            drawGL(&character, 1);
        }
        
        initGL(characters.begin()->getObject(), 0);
        for (auto &character : characters) {
            drawGL(&character, 0);
            character.update(glfwGetTime() - last);
            if (character.getPosition().x < -10 || character.getPosition().z < -10 ||
                character.getPosition().x > 10 || character.getPosition().z > 10)
                character.setVelocity(glm::vec3(0) - character.getVelocity());
            bool flag = character.getCFlag();
            if (sampler->sample(position, &character) == Collision::status::COLLISION && !flag) {
                numObj--;
            }
            
            for (auto &character2 : characters) {
                if (&character != &character2)
                    sampler->sampleCharacters(&character, &character2);
            }
        }
        
        drawGround();
        
        char text[256];
        
        sprintf(text,"%.2f FPS", frames / glfwGetTime() );
        printText2D(text, 10, 60, 20);
        
        sprintf(text, "Score: %d", sampler->getScore());
        printText2D(text, 620, 60, 20);
        
        sprintf(text, "Objects Left: %d", numObj);
        printText2D(text, 480, 575, 20);
        
        if (numObj == 0) {
            sprintf(text, "YOU WON!");
            printText2D(text, 175, 290, 60);
        } else {
            last = elapsed;
            elapsed = glfwGetTime() - start;
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frames++;
    }
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );
    
    glfwTerminate();
    
    return 0;
}
