#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include "GLSLProgram.h"
#include "GLSL.h"

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
}

bool CubeMapTexture::loadCubeMapSide(GLuint texture, GLenum side, const char* fileName) {
	glBindTexture (GL_TEXTURE_CUBE_MAP, texture);

	int x, y, n;
	int force_channels = 4;
	unsigned char*  image_data = stbi_load (
	fileName, &x, &y, &n, force_channels);
	
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
		void render(glm::mat4, glm::mat4, glm::vec3);

	private:
		CubeMapTexture *cubeMap;
		GLuint posBuf;
		GLuint textureObj;

		GLuint program;
		GLuint posID;
		GLuint viewMatID;
		GLuint projMatID;
		GLuint modelMatID;
		GLuint texSamplerID;
};

Skybox::Skybox(GLuint prog) {
	cubeMap = new CubeMapTexture("../Assets/models/hourglass_right.jpg",
								"../Assets/models/hourglass_left.jpg",
								"../Assets/models/hourglass_top.jpg",
								"../Assets/models/hourglass_top.jpg",
								"../Assets/models/hourglass_front.jpg",
								"../Assets/models/hourglass_back.jpg",
								&textureObj);

	program = prog;

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

	glGenBuffers(1, &posBuf);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertex_buffer_data), s_vertex_buffer_data, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLSL::checkVersion();
    assert(glGetError() == GL_NO_ERROR);
}

void Skybox::prepareForRender() {
	cubeMap->bind(textureObj);
}

void Skybox::initShaderVars() {
	glUseProgram(program);
	posID = glGetAttribLocation(program, "aPos");
	texSamplerID = glGetUniformLocation(program, "cubeMapTexture");
	projMatID = glGetUniformLocation(program, "P");
	viewMatID = glGetUniformLocation(program, "V");
	modelMatID = glGetUniformLocation(program, "M");
}

void Skybox::render(glm::mat4 view, glm::mat4 proj, glm::vec3 position) {
	glEnable(GL_DEPTH_TEST);
	glUseProgram(program);
	prepareForRender();

	glm::mat4 tempMat = glm::mat4(1.0);
	vec3 pos = vec3(position.x, 0, position.y);
	tempMat = glm::translate(tempMat, pos);
	tempMat = glm::scale(tempMat, vec3(100, 100, 100));

	glEnableVertexAttribArray(posID);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glVertexAttribPointer(posID, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glUniformMatrix4fv(viewMatID, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projMatID, 1, GL_FALSE, glm::value_ptr(proj));
	glUniformMatrix4fv(modelMatID, 1, GL_FALSE, glm::value_ptr(tempMat)); 
	glUniform1i(texSamplerID, 0);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDisableVertexAttribArray(0);
	assert(glGetError() == GL_NO_ERROR);
}
#endif
