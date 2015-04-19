class HUD
{
public:

static void drawHud(float x, float y, glm::vec3 color, void* font, char string[])
	{
	   glColor3f(color.r, color.g, color.b);
	   glRasterPos2f(x, y);
	   for (int i = 0; i < strlen(string); i++) {
	      glutBitmapCharacter(font, string[i]);
	   }
	}
}