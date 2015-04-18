#include "glad\glad.h"

#include <sstream>
#include <cstdio>
#include <ctime>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include "GLSL.h"
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "RenderingHelper.h"

#include "Camera.h"
#include "Object.h"
#include "TimeManager.h"
#include "HUD.h"

// Include AntTweakBar
#include "AntTweakBar-1.16\include\AntTweakBar.h"

#define NUM_MESHES 15
#define TRANS_SCALE 3
#define VELOCITY 0.2

GLFWwindow* window;
using namespace std;
using namespace glm;
using namespace tinyobj;

vector<shape_t> sphere;
vector<material_t> materials;

int g_width, g_height;
bool free_cam;
float g_Camtrans = -2.5;
vec3 g_light(0, 40, 0);

Camera g_camera = Camera(vec3(0, 10, 0), 0.0f, -0.81f);
mat4 ProjectionMatrix, ViewMatrix;

float mouseSpeed = 0.005f;
float speed = 0.05f;

float g_x, g_y, g_z;

int game_score = 0;

bool score_printed = true;

//declare a matrix stack
RenderingHelper ModelTrans;

GLuint ShadeProg;

vector<Object> sphereShapes;

GLuint posBufObjG = 0;
GLuint norBufObjG = 0;

//Handles to the shader data
GLint h_aPosition;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uLightPos;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;


//Randomly generate a number between a high and low value (-1 and 1)
float rand_num(float lo, float hi) {
   return lo + (float)((hi - lo) * rand()) / (float)RAND_MAX;
}

/* helper function to make sure your matrix handle is correct */
inline void safe_glUniformMatrix4fv(const GLint handle, const GLfloat data[]) {
   if (handle >= 0)
      glUniformMatrix4fv(handle, 1, GL_FALSE, data);
}

/* helper function to send materials to the shader - you must create your own */
void SetMaterial(int i) {

   glUseProgram(ShadeProg);
   switch (i) {
   case 0: //grass green
      glUniform3f(h_uMatAmb, 0.00, 0.2 , 0.00);
      glUniform3f(h_uMatDif, 0.0, 0.5, 0.0);
      glUniform3f(h_uMatSpec, 0.00, 0.1, 0.00);
      glUniform1f(h_uMatShine, 1.0);
      break;
   case 1: // black
      glUniform3f(h_uMatAmb, 0.03, 0.03, 0.03);
      glUniform3f(h_uMatDif, 0.3, 0.3, 0.3);
      glUniform3f(h_uMatSpec, 0.03, 0.03, 0.03);
      glUniform1f(h_uMatShine, 20.0);
      break;
   case 2: // red
      glUniform3f(h_uMatAmb, 0.8, 0.05, 0.05);
      glUniform3f(h_uMatDif, 0.9, 0.1, 0.1);
      glUniform3f(h_uMatSpec, 0.1, 0.01, 0.01);
      glUniform1f(h_uMatShine, 2.0);
      break;
   }
}

/* helper function to set projection matrix - don't touch */
void SetProjectionMatrix() {
   mat4 Projection = perspective(90.0f, (float)g_width / g_height, 0.1f, 100.f);
   safe_glUniformMatrix4fv(h_uProjMatrix, value_ptr(Projection));
}

/* camera controls - do not change beyond the current set up to rotate*/
void SetView() {
   // Get mouse position
   double xpos, ypos;
   glfwGetCursorPos(window, &xpos, &ypos);

   if (free_cam) {

      // glfwGetTime is called only once, the first time this function is called
      static double lastTime = glfwGetTime();

      // Compute time difference between current and last frame
      double currentTime = glfwGetTime();
      float deltaTime = float(currentTime - lastTime);

      // Reset mouse position for next frame
      glfwSetCursorPos(window, g_width / 2, g_height / 2);

      // Compute new orientation
      g_camera.setTheta(g_camera.getTheta() + mouseSpeed * float(g_width / 2 - xpos));
      g_camera.setPhi(g_camera.getPhi() + mouseSpeed * float(g_height / 2 - ypos));

      //between 89 and -89 degrees
      g_camera.setPhi(max(g_camera.getPhi(), -1.56f));
      g_camera.setPhi(min(g_camera.getPhi(), 1.56f));
   }
   //cout << g_camera.getPosition().x << " " << g_camera.getPosition().y << " " << g_camera.getPosition().z << endl;

   //cout << "theta: " << g_camera.getTheta() << " phi: " << g_camera.getPhi() << endl;

   //Direction: Spherical coordinates to Cartesian coordinates conversion
   vec3 direction(cos(g_camera.getPhi()) * sin(g_camera.getTheta()), sin(g_camera.getPhi()), cos(g_camera.getPhi()) * cos(g_camera.getTheta()));

   //Right vector
   vec3 rightV = vec3(sin(g_camera.getTheta() - 3.14f / 2.0f), 0, cos(g_camera.getTheta() - 3.14f / 2.0f));

   //Up vector
   vec3 upV = cross(rightV, direction);

   if (free_cam) {
      // Move forward
      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
         g_camera.setPosition(g_camera.getPosition() + direction * speed);
      }
      // Move backward
      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
         g_camera.setPosition(g_camera.getPosition() - direction * speed);
      }
      // Strafe right
      if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
         g_camera.setPosition(g_camera.getPosition() + rightV * speed);
      }
      // Strafe left
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
         g_camera.setPosition(g_camera.getPosition() - rightV * speed);
      }

      g_camera.setY(max(g_camera.getY(), 0.5f));

   }
   ViewMatrix = lookAt(
      g_camera.getPosition(),           // Camera is here
      g_camera.getPosition() + direction, // and looks here : at the same position, plus "direction"
      upV                  // Head is up (set to 0,-1,0 to look upside-down)
      );
   safe_glUniformMatrix4fv(h_uViewMatrix, value_ptr(ViewMatrix));
}

/* model transforms */
void SetModel(vec3 trans, float rot, float sc) {
   mat4 Trans = translate(mat4(1.0f), trans);
   mat4 RotateY = rotate(mat4(1.0f), rot, vec3(0.0f, 1, 0));
   mat4 Sc = scale(mat4(1.0f), vec3(sc));
   mat4 com = Trans*RotateY*Sc;
   safe_glUniformMatrix4fv(h_uModelMatrix, value_ptr(com));
}

void initGround() {

   float G_edge = 5;
   GLfloat g_backgnd_data[] = {
      -G_edge, 0.0f, -G_edge,
      -G_edge, 0.0f, G_edge,
      G_edge, 0.0f, -G_edge,
      -G_edge, 0.0f, G_edge,
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

void initGL() {
   vec3 position;

   // Set the background color
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
   // Enable Z-buffer test
   glEnable(GL_DEPTH_TEST);

   //initialize the ModelTrans matrix stack
   ModelTrans.useModelViewMatrix();
   ModelTrans.loadIdentity();

   for (int i = 0; i < NUM_MESHES; i++) {
      position = vec3(i * 3, 1, 0);

      sphereShapes.push_back(Object(position, vec3(rand_num(-1, 1), 0, rand_num(-1, 1)), VELOCITY, AABB(position, vec3(0.9, 0.9, 0.9))));
      sphereShapes.at(i).loadShapes("sphere.obj");
      //cout << sphereShapes.at(i).position.x << " " << sphereShapes.at(i).position.y << " " << sphereShapes.at(i).position.z << endl;
      sphereShapes.at(i).initShape();
   }

   initGround();
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
   if (!rc) {
      printf("Error compiling vertex shader %s\n", vShaderName.c_str());
      return false;
   }

   // Compile fragment shader
   glCompileShader(FS);
   GLSL::printError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
   GLSL::printShaderInfoLog(FS);
   if (!rc) {
      printf("Error compiling fragment shader %s\n", fShaderName.c_str());
      return false;
   }

   // Create the program and link
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);
   glLinkProgram(ShadeProg);

   GLSL::printError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &rc);
   GLSL::printProgramInfoLog(ShadeProg);
   if (!rc) {
      printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
      return false;
   }

   /* get handles to attribute data */
   h_aPosition = GLSL::getAttribLocation(ShadeProg, "aPos");
   h_aNormal = GLSL::getAttribLocation(ShadeProg, "aNor");
   h_uProjMatrix = GLSL::getUniformLocation(ShadeProg, "uProjM");
   h_uViewMatrix = GLSL::getUniformLocation(ShadeProg, "uViewM");
   h_uModelMatrix = GLSL::getUniformLocation(ShadeProg, "uModelM");
   h_uLightPos = GLSL::getUniformLocation(ShadeProg, "uLightPos");
   h_uMatAmb = GLSL::getUniformLocation(ShadeProg, "UaColor");
   h_uMatDif = GLSL::getUniformLocation(ShadeProg, "UdColor");
   h_uMatSpec = GLSL::getUniformLocation(ShadeProg, "UsColor");
   h_uMatShine = GLSL::getUniformLocation(ShadeProg, "Ushine");

   assert(glGetError() == GL_NO_ERROR);
   return true;
}

void drawGround() {
   //draw the ground
   SetModel(vec3(0, 0, 0), 0, 10);
   SetMaterial(0);
   glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glDrawArrays(GL_TRIANGLES, 0, 2 * 3);
   GLSL::disableVertexAttribArray(h_aPosition);
   GLSL::disableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void drawGL() {
   // Clear the screen
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Use our GLSL program
   glUseProgram(ShadeProg);

   SetProjectionMatrix();
   SetView();

   glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);

   //draw the ground
   drawGround();

   game_score = 0;

   for (int i = 0; i < sphereShapes.size(); i++) {
      SetMaterial(1);

      AABB cameraBB = AABB(g_camera.getPosition(), vec3(0.1, 0.1, 0.1));
      if (!sphereShapes.at(i).touched) {
         if (cameraBB.testCollision(sphereShapes.at(i).boundingBox)) {
            sphereShapes.at(i).touched = true;
            score_printed = false;
         }
      }

      if (sphereShapes.at(i).touched) {
         SetMaterial(2);
         game_score++;
      }

      for (int j = 0; j < sphereShapes.size(); j++) {
         if (sphereShapes.at(i).boundingBox.testCollision(sphereShapes.at(j).boundingBox)) {
            sphereShapes.at(i).direction = -sphereShapes.at(i).direction;
            sphereShapes.at(j).direction = -sphereShapes.at(j).direction;
         }
      }

      ModelTrans.loadIdentity();

      ModelTrans.translate(sphereShapes.at(i).position);
      sphereShapes.at(i).draw(h_aPosition, h_aNormal, h_uModelMatrix, ModelTrans);

      sphereShapes.at(i).timeStep(TimeManager::Instance().DeltaTime);
   }

   if (!score_printed) {
      cout << "Score: " << game_score << endl;
      score_printed = true;
   }

   // Disable and unbind
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glUseProgram(0);
   assert(glGetError() == GL_NO_ERROR);
}

void window_size_callback(GLFWwindow* window, int w, int h) {
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   g_width = w;
   g_height = h;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (key == GLFW_KEY_Q && (action == GLFW_REPEAT || action == GLFW_PRESS))
      g_light.x += 0.25;
   if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS))
      g_light.x -= 0.25;
   //F KEY - Put the camera in free cam mode or take it out of free cam mode
   if (key == GLFW_KEY_F && action == GLFW_PRESS) {
      free_cam = !free_cam;
   }
}

int main(int argc, char **argv) {
   // Initialise GLFW
   if (!glfwInit())
   {
      fprintf(stderr, "Failed to initialize GLFW\n");
      return -1;
   }

   glfwWindowHint(GLFW_SAMPLES, 4);
   glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

   // Open a window and create its OpenGL context
   g_width = 1024;
   g_height = 768;
   window = glfwCreateWindow(g_width, g_height, "Lab 1", NULL, NULL);
   if (window == NULL) {
      fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
      glfwTerminate();
      return -1;
   }
   glfwMakeContextCurrent(window);
   glfwSetKeyCallback(window, key_callback);
   glfwSetWindowSizeCallback(window, window_size_callback);
   // Initialize glad
   if (!gladLoadGL()) {
      fprintf(stderr, "Unable to initialize glad");
      glfwDestroyWindow(window);
      glfwTerminate();
      return 1;
   }

   // Ensure we can capture the escape key being pressed below
   glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   initGL();
   installShaders("vert.glsl", "frag.glsl");

   TwInit(TW_OPENGL, NULL);
   TwWindowSize(1024, 768);
   TwBar * GUI = TwNewBar("Game Score");
   TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
   std::string message = "PLZZ";
   TwAddVarRW(GUI, "Score: ", TW_TYPE_STDSTRING, &message, NULL);

   do{
      // Use our Singleton to calculate our framerate every frame, passing true to set FPS in titlebar
      TimeManager::Instance().CalculateFrameRate(true);

      drawGL();
      std::ostringstream oss;
      oss << game_score;
      message = oss.str();

      TwDraw();
      // Swap buffers
      glfwSwapBuffers(window);
      glfwPollEvents();
   } // Check if the ESC key was pressed or the window was closed
   while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
   glfwWindowShouldClose(window) == 0);

   // Close OpenGL window and terminate GLFW
   glfwTerminate();

   return 0;
}
