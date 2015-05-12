#ifndef BILLBOARD_H
#define BILLBOARD_H
#define NUM_CLOUDS 100

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "helper.h"
#include "GLSL.h"
#include "Camera.hpp"
#include <SOIL/SOIL.h>
#include <png.h>

class Billboard {
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
		float xtrans[NUM_CLOUDS];
		float ztrans[NUM_CLOUDS];

		//GLuint loadTexture();
		float randNum();
	public:
		Billboard();
		void draw(Camera*);
};

Billboard::Billboard() {
	GLint rc;
    GLSL::printError(__FILE__,__LINE__);

    // Create Vertex Array Object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create Vertex Buffer Object
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);

	/*glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);*/
    
    // Create shader handles
    GLuint VS = glCreateShader(GL_VERTEX_SHADER);
    GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
    GLSL::printError(__FILE__,__LINE__);
    
    // Read shader sources
    const char *vshader = GLSL::textFileRead("shd/cloud_vert.glsl");
    const char *fshader = GLSL::textFileRead("shd/cloud_frag.glsl");
    glShaderSource(VS, 1, &vshader, NULL);
    glShaderSource(FS, 1, &fshader, NULL);
    GLSL::printError(__FILE__,__LINE__);
    
    // Compile vertex shader
    glCompileShader(VS);
    GLSL::printError(__FILE__,__LINE__);
    glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
    GLSL::printShaderInfoLog(VS);
    if(!rc) {
        printf("Error compiling vertex shader %s\n", "shd/cloud_vert.glsl");
    }

    // Compile fragment shader
    glCompileShader(FS);
    GLSL::printError(__FILE__,__LINE__);
    glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
    GLSL::printShaderInfoLog(FS);
    if(!rc) {
        printf("Error compiling fragment shader %s\n", "shd/cloud_frag.glsl");
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
        printf("Error linking shaders %s and %s\n", "shd/cloud_vert.glsl", "shd/cloud_frag.glsl");
    }

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
	for (i = 0; i < NUM_CLOUDS; i++) {
		xtrans[i] = randNum();
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

void Billboard::draw(Camera *camera) {
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
	
	// Send texture array to the GPU
	glEnableVertexAttribArray(texAttrib);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind the element array to the GPU
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(texSamplerID, 0);

	int i;
	for (i = 0; i < NUM_CLOUDS; i++) {
		glUniform3f(BillboardPosID, 200.0f + xtrans[i], 100.0f, 200.0f + ztrans[i]);
		glUniform2f(BillboardSizeID, 20.0f, 20.0f);

		/*glEnableVertexAttribArray(posAttrib);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);*/
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	glDisableVertexAttribArray(posAttrib);
	glDisableVertexAttribArray(texAttrib);
	//glDisableVertexElementArray(ebo);
	glUseProgram(0);
	GLSL::printError(__FILE__, __LINE__);
	assert(glGetError() == GL_NO_ERROR);
}

float Billboard::randNum() {
    return ((float) rand() / (RAND_MAX)) * 600.0;
}

/*GLuint loadTexture(const string filename, int &width, int &height) {
   //header for testing if it is a png
   png_byte header[8];
 
   //open file as binary
   FILE *fp = fopen(filename.c_str(), "rb");
   if (!fp) {
     return TEXTURE_LOAD_ERROR;
   }
 
   //read the header
   fread(header, 1, 8, fp);
 
   //test if png
   int is_png = !png_sig_cmp(header, 0, 8);
   if (!is_png) {
     fclose(fp);
     return TEXTURE_LOAD_ERROR;
   }
 
   //create png struct
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
       NULL, NULL);
   if (!png_ptr) {
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }
 
   //create png info struct
   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
     png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }
 
   //create png info struct
   png_infop end_info = png_create_info_struct(png_ptr);
   if (!end_info) {
     png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }
 
   //png error stuff, not sure libpng man suggests this.
   if (setjmp(png_jmpbuf(png_ptr))) {
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     fclose(fp);
     return (TEXTURE_LOAD_ERROR);
   }
 
   //init png reading
   png_init_io(png_ptr, fp);
 
   //let libpng know you already read the first 8 bytes
   png_set_sig_bytes(png_ptr, 8);
 
   // read all the info up to the image data
   png_read_info(png_ptr, info_ptr);
 
   //variables to pass to get info
   int bit_depth, color_type;
   png_uint_32 twidth, theight;
 
   // get info about png
   png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type,
       NULL, NULL, NULL);
 
   //update width and height based on png info
   width = twidth;
   height = theight;
 
   // Update the png info struct.
   png_read_update_info(png_ptr, info_ptr);
 
   // Row size in bytes.
   int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
 
   // Allocate the image_data as a big block, to be given to opengl
   png_byte *image_data = new png_byte[rowbytes * height];
if (!image_data) {
     //clean up memory and close stuff
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     fclose(fp);
     return TEXTURE_LOAD_ERROR;
   }
 
   //row_pointers is for pointing to image_data for reading the png with libpng
   png_bytep *row_pointers = new png_bytep[height];
   if (!row_pointers) {
     //clean up memory and close stuff
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     delete[] image_data;
     fclose(fp);
     return TEXTURE_LOAD_ERROR;
   }
   // set the individual row_pointers to point at the correct offsets of image_data
   for (int i = 0; i < height; ++i)
     row_pointers[height - 1 - i] = image_data + i * rowbytes;
 
   //read the png into image_data through row_pointers
   png_read_image(png_ptr, row_pointers);
 
   //Now generate the OpenGL texture object
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA, width, height, 0,
       GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) image_data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 
   //clean up memory and close stuff
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   delete[] image_data;
   delete[] row_pointers;
   fclose(fp);
 
   return texture;
 }*/
#endif
