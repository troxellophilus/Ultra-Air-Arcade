#ifndef DRAWTEXT_H
#define DRAWTEXT_H

#include <GL/glew.h>

#ifdef __APPLE__
    #define GLFW_INCLUDE_GLCOREARB
#endif

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "Text.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

//Struct to help with setting font colors 
struct GLfloat4 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;

    GLfloat4(GLfloat a, GLfloat b, GLfloat c, GLfloat d) {
        x = a;
        y = b;
        z = c;
        w = d;
    }
};

//Struct to help define the dimensions of a character
struct point {
    GLfloat x;
    GLfloat y;
    GLfloat s;
    GLfloat t;
};

struct MyFont {
    std::string font;
    unsigned char *fontBuffer;
    int size;

    MyFont(std::string f, unsigned char *buf, int s) {
        font = f;
        fontBuffer = buf;
        size = s;
    }
};

class DrawText {
private:
    //vector of Text that will be drawn
    std::vector<Text> texts;
    std::vector<std::string> fontnames;
    std::vector<MyFont> fonts;

public:
    int width, height;
    //Constructor/destructor
    DrawText();
    virtual ~DrawText();

    //Functions
    int initResources(int w, int h, GLint coord, GLint text, GLint color);
    void initFonts();
    void destructFonts();
    const char *setFont(int f);
    GLfloat4 setFontColor(int c);
    int getFontSize(int divisor);
    void addText(Text text);
    void renderText(const char *text, int font, float x, float y, float sx, float sy);
    void drawText();
    // void countdown(int seconds);
};


#endif