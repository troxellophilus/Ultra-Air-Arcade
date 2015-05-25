#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include "GLSLProgram.h"
#include "GLSL.h"
//#include <Magick++/Include.h>
#include <stb_image.h>
#include <SOIL/SOIL.h>

using namespace std;

class CubeMapTexture {
	public:
		CubeMapTexture(const char*,
		const char*,
		const char*,
		const char*,
		const char*,
		const char*,
		GLuint*);
		~CubeMapTexture();
		//bool load();
		void bind(GLuint);
		bool loadCubeMapSide(GLuint texture, GLenum side, const char* fileName);
		string fileNames[6];
};

CubeMapTexture::CubeMapTexture(const char* posXFileName,
	const char* negXFileName,
	const char* posYFileName, 
	const char* negYFileName,
	const char* posZFileName,
	const char* negZFileName,
	GLuint *texCube) {
	
	glActiveTexture (GL_TEXTURE0);
	glGenTextures (1, texCube);

	loadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, negZFileName);
	loadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, posZFileName);
	loadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, posYFileName);
	loadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, negYFileName);
	loadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, negXFileName);	
	loadCubeMapSide(*texCube, GL_TEXTURE_CUBE_MAP_POSITIVE_X, posXFileName);

	// format cube map texture
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	fprintf(stderr, "1\n");
}

bool CubeMapTexture::loadCubeMapSide(GLuint texture, GLenum side, const char* fileName) {
	glBindTexture (GL_TEXTURE_CUBE_MAP, texture);

	int x, y, n;
	int force_channels = 4;
	unsigned char*  image_data = stbi_load (
	fileName, &x, &y, &n, force_channels);
	/*unsigned char* image_data = SOIL_load_image(fileName, &x, &y, &n, SOIL_LOAD_AUTO);*/
	fprintf(stderr, "%d\n", image_data);
	
	if (!image_data) {
		fprintf (stderr, "ERROR: could not load %s\n", fileName);
		return false;
	}
	// non-power-of-2 dimensions check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf (stderr, "WARNING: image %s is not power-of-2 dimensions\n", fileName);
	}

	// copy image data into 'target' side of cube map
	glTexImage2D (
		side,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
	);
	free (image_data);
	return true;
}

CubeMapTexture::~CubeMapTexture() {
	/*if (textureObj != 0)
		glDeleteTextures(1, &textureObj);*/
	;
}

void CubeMapTexture::bind(GLuint texObj) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texObj);
}

class Skybox {
	public:
		Skybox(GLuint);
		~Skybox();
		void prepareForRender();
		void initShaderVars();
		void render(glm::mat4, glm::mat4);

	private:
		CubeMapTexture *cubeMap;
		GLuint posBuf;
		GLuint textureObj;

		GLuint progID;
		GLuint posID;
		GLuint viewMatID;
		GLuint projMatID;
		GLuint texSamplerID;
};

Skybox::Skybox(GLuint prog) {
	cubeMap = new CubeMapTexture("../Assets/models/desert_right.jpg",
								"../Assets/models/desert_left.jpg",
								"../Assets/models/desert_top.jpg",
								"../Assets/models/desert_top.jpg",
								"../Assets/models/desert_front.jpg",
								"../Assets/models/desert_back.jpg",
								&textureObj);
	//cubeMap->load();
	progID = prog;

	static const GLfloat s_vertex_buffer_data[] = {
		-10.0f,  10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,

		-10.0f, -10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		-10.0f,  10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f,  10.0f
	};
	fprintf(stderr, "2\n");
	glGenBuffers(1, &posBuf);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertex_buffer_data), s_vertex_buffer_data, GL_STATIC_DRAW);
	
	fprintf(stderr, "3\n");
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLSL::checkVersion();
    assert(glGetError() == GL_NO_ERROR);
}

void Skybox::prepareForRender() {
	fprintf(stderr, "4\n");
	cubeMap->bind(textureObj);
	fprintf(stderr, "5\n");
}

void Skybox::initShaderVars() {
	fprintf(stderr, "6\n");
	glUseProgram(progID);
	posID = glGetAttribLocation(progID, "aPos");
	texSamplerID = glGetUniformLocation(progID, "cubeMapTexture");
	projMatID = glGetUniformLocation(progID, "P");
	viewMatID = glGetUniformLocation(progID, "V");
	fprintf(stderr, "7\n");
}

void Skybox::render(glm::mat4 view, glm::mat4 proj) {
	fprintf(stderr, "8\n");
	glUseProgram(progID);
	prepareForRender();

	glEnableVertexAttribArray(posID);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glVertexAttribPointer(posID, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glUniformMatrix4fv(viewMatID, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projMatID, 1, GL_FALSE, glm::value_ptr(proj));
	glUniform1i(texSamplerID, 0);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDisableVertexAttribArray(0);
	fprintf(stderr, "9\n");
}
#endif
