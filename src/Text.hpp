//Class to define a Text object.
class Text {
private:
    const char *text; // String of text to draw on screen
    int pos_x;  // x dimension position
    int pos_y; // y dimension position
    float angle; // angle of text in degrees
    int font; // font selection
    int font_size; // size of the font
    int font_color; // color of the font

public:
    Text(const char *t, int x, int y, float deg, int fontface, int size, int color) {
        text = t;
        pos_x = x; 
        pos_y = y;
        angle = deg;
        font = fontface;
        font_size = size;
        font_color = color;
    }

    const char *getText() {
        return text;
    }

    int getX() {
        return pos_x;
    }

    int getY() {
        return pos_y;
    }

    int getAngle() {
        return angle;
    }

    int getFont() {
        return font;
    } 

    int getSize() {
        return font_size;
    }

    int getColor() {
        return font_color;
    }
};