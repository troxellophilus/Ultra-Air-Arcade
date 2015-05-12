#include "DrawText.h"

GLuint vbo;
GLint aCoord;
GLint uText;
GLint uColor;


FT_Library ft;
FT_Face face;

DrawText::DrawText() {
    fontnames.push_back("Fonts/FreeSans.ttf");
    fontnames.push_back("Fonts/Vera.ttf");
    fontnames.push_back("Fonts/Windsong.ttf");
    fontnames.push_back("Fonts/helsinki.ttf");
}

DrawText::~DrawText() {

}

/*
 * Initialize memory for the fonts.
 */
void DrawText::initFonts() {
    for (int i = 0; i < fontnames.size(); i++) {
              // Open up a font file.
       std::ifstream fontFile(fontnames.at(i), std::ios::binary);
       if (fontFile) {
            // Read the entire file to a memory buffer.
            fontFile.seekg(0, std::ios::end);
            std::fstream::pos_type fontFileSize = fontFile.tellg();
            fontFile.seekg(0);
            unsigned char *fontBuffer = new unsigned char[fontFileSize];
            fontFile.read((char *)fontBuffer, fontFileSize);

            MyFont newFont = MyFont(fontnames.at(i), fontBuffer, fontFileSize);
            fonts.push_back(newFont);
        }
    }
}

/*
 * Release the memory held by the fonts.
 */
void DrawText::destructFonts() {
    for (int i = 0; i < fonts.size(); i++) {
        delete [] fonts.at(i).fontBuffer;
    }
}

/*
 * Initialize Freetype, the fonts, and the text shader.
 */
int DrawText::initResources(int w, int h, GLint coord, GLint text, GLint color) {
	width = w;
	height = h;

    /* Initialize the FreeType2 library */
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Could not init freetype library\n");
        return 0;
    }

    aCoord = coord;
    uText = text;
    uColor = color;

    initFonts();

    // Create the vertex buffer object
    glGenBuffers(1, &vbo);

    return 1;
}

/* 
 * Selection of a font color 
 */
GLfloat4 DrawText::setFontColor(int c) {
    GLfloat4 color = GLfloat4(0, 0, 0, 1);
    switch(c) {
        case 0: //black
            color = GLfloat4(0, 0, 0, 1);
            break;
        case 1: //red
            color = GLfloat4(1, 0, 0, 1);
            break;
        case 2: //transparent green
            color = GLfloat4(0, 1, 0, 0.5);
            break;
        default:
            break;
    }

    return color;
}

/*
 * Create a font size relative to the size of the screen
 */
int DrawText::getFontSize(int divisor) {
    return (width + height) / divisor;
}

/*
 * Add texts to the vector of texts
 */
void DrawText::addText(Text text) {
	texts.push_back(text);
}

/*
 * Render text using the currently loaded font and currently set font size.
 * Rendering starts at coordinates (x, y), z is always 0.
 * The pixel coordinates that the FreeType2 library uses are scaled by (sx, sy).
 */
void DrawText::renderText(const char *text, int font, float x, float y, float sx, float sy) {
    const char *p;

    FT_GlyphSlot slot = face->glyph;

    /* Create a texture that will be used to hold one "glyph" */
    GLuint tex;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(uText, 0);

    /* We require 1 byte alignment when uploading texture data */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Clamping to edges is important to prevent artifacts when scaling */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Linear filtering usually looks best for text */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Set up the VBO for our vertex data */
    glEnableVertexAttribArray(aCoord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(aCoord, 4, GL_FLOAT, GL_FALSE, 0, 0);

    FT_Matrix matrix;
    FT_Vector     pen;                    /* untransformed origin  */
    double ang = ( 25.0 / 360 ) * 3.14159 * 2;      /* use 25 degrees     */

    /* Transform matrix */
    matrix.xx = (FT_Fixed)( cos( 0 ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( 0 ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( 0 ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( 0 ) * 0x10000L );

    pen.x = x * 64;
    pen.y = ( height - y ) * 64;

    /* Loop through all characters */
    for (p = text; *p; p++) {
        // FT_Set_Transform( face, &matrix, &pen );

        /* Try to load and render the character */
        if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
            continue;

        /* Upload the "bitmap", which contains an 8-bit grayscale image, as an alpha texture */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, slot->bitmap.width, slot->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, slot->bitmap.buffer);

        /* Calculate the vertex and texture coordinates */
        float x2 = x + slot->bitmap_left * sx;
        float y2 = -y - slot->bitmap_top * sy;
        float w = slot->bitmap.width * sx;
        float h = slot->bitmap.rows * sy;

        point box[4] = {
            {x2, -y2, 0, 0},
            {x2 + w, -y2, 1, 0},
            {x2, -y2 - h, 0, 1},
            {x2 + w, -y2 - h, 1, 1},
        };

        /* Draw the character on the screen */
        glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /* Advance the cursor to the start of the next character */
        x += (slot->advance.x >> 6) * sx;
        y += (slot->advance.y >> 6) * sy;
    }

    glDisableVertexAttribArray(aCoord);
    glDeleteTextures(1, &tex);
}

/*
 * Set up the text renderer to draw all of the texts sent by the main
 * function.
 */
void DrawText::drawText() {
	float sx = 2.0 / width;
	float sy = 2.0 / height;

	/* Black background */
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Enable blending, necessary for our alpha texture */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int i;
    for (i = 0; i < texts.size(); i++) {
        /* Load a font */
        if(FT_New_Memory_Face(ft, fonts.at(texts[i].getFont()).fontBuffer, fonts.at(texts[i].getFont()).size, 0, &face)) {
		    fprintf(stderr, "Could not open font %d\n", texts[i].getFont());
		}

        /* Set font size and color */
        FT_Set_Pixel_Sizes(face, 0, texts[i].getSize());
        GLfloat4 fC4 = setFontColor(texts[i].getColor());
        GLfloat fontColor[4] = {fC4.x, fC4.y, fC4.z, fC4.w};
        glUniform4fv(uColor, 1, fontColor);

        /* Convert the world space coordinates to clip space coordinates */
        float new_x = (float)texts[i].getX() / width - sx - 0.5;
        float new_x1 = new_x * 2;
        float new_y = (float)texts[i].getY() / height - sy - 0.5;
        float new_y1 = new_y * 2;
        renderText(texts[i].getText(), texts[i].getFont(), new_x1, new_y1, sx, sy);
        // printf("%s at x: %f, y: %f with font %d and color %d\n", texts[i].getText(), new_x1, new_y1, texts[i].getFont(), texts[i].getColor());
        FT_Done_Face( face );
    }

    /* Clear the vector of texts so the next pass will have a refreshed list */
    texts.clear();
}