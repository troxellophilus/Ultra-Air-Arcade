#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include "GLSLProgram.h"

#include <ImageMagick-6/Magick++.h>

class CubeMapTexture {
	public:
		CubeMapTexture(const string& posXFilename,
		const string& negXFilename,
		const string& posYFilename,
		const string& negYFilename,
		const string& posZFilename,
		const string& negZFilename);

		~CubemapTexture();
		bool load();
		void bind(GLenum TextureUnit);
	private:
		string fileNames[6];
		GLuint textureObj;
};

CubeMapTexture::CubeMapTexture(const string& posXFileName,
	const string& negXFileName,
	const string& posYFileName, 
	const string& negYFileName,
	const string& posZFileName,
	const string& negZFileName) {
	fileNames[0] = posXFileName;
	fileNames[1] = negXFIleName;
	fileNames[2] = posYFileName;
	fileNames[3] = negYFileName;
	fileNames[4] = posZFileName;
	fileNames[5] = negZFileName;

	textureObj = 0;
}

CubeMapTexture::~CubeMapTexture() {
	if (textureObj != 0)
		glDeleteTextures(1, &textureObj);
}

bool CubeMapTexture::load() {
    glGenTextures(1, &textureObj);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureObj);

    Magick::Image* pImage = NULL;
    Magick::Blob blob;

    for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(types) ; i++) {
        pImage = new Magick::Image(fileNames[i]);

        try {            
            pImage->write(&blob, "RGBA");
        }
        catch (Magick::Error& Error) {
            cout << "Error loading texture '" << m_fileNames[i] << "': " << Error.what() << endl;
            delete pImage;
            return false;
        }

        glTexImage2D(types[i], 0, GL_RGB, pImage->columns(), pImage->rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, blob.data());

        delete pImage;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return true;

}

void CubeMapTexture::bind(GLenum TextureUnit) {
	glActiveTexture(TextureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureObj);
}

class Skybox {
	public:
		Skybox();
		~Skybox();

	private:
		CubeMapTexture cubeMap;
};
#endif
