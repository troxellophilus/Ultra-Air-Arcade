#ifndef DRAW_H
#define DRAW_H

using namespace std;

void drawPawn(int rank, int file, int material, int rot);
void drawKing(int rank, int file, int material, int rot);
void drawQueen(int rank, int file, int material, int rot);
void drawBishop(int rank, int file, int material, int rot);
void drawKnight(int rank, int file, int material, int rot);
void drawRook(int rank, int file, int material);


//Class to represent a game piece
class Piece {
public:
   char* type; //what kind of piece it is (pawn, rook, etc.)
   int rank;
   int file;
   int material; //black or white
   int rotation; 
   int draw; //whether or not to draw the piece
   int selected; //whether or not the piece is selected

   Piece() {}

   Piece(char* t, int r, int f, int m, int rot, int d) {
      type = t;
      rank = r;
      file = f;
      material = m;
      rotation = rot;
      draw = d;
      selected = 0;
   }

   void setRank(int r) {
      rank = r; 
   }
   
   void setFile(int f) {
      file = f;
   }

   int getRank() {
      return rank;
   }

   int getFile() {
      return file;
   }

   //Draw the piece based on the type
   void drawPiece() {
      if (draw) {
         if (!strcmp("pawn", type))
            drawPawn(rank, file, material, rotation);
         else if (!strcmp("rook", type))
            drawRook(rank, file, material);
         else if (!strcmp("knight", type))
            drawKnight(rank, file, material, rotation);
         else if (!strcmp("bishop", type))
            drawBishop(rank, file, material, rotation);
         else if (!strcmp("queen", type))
            drawQueen(rank, file, material, rotation);
         else if (!strcmp("king", type))
            drawKing(rank, file, material, rotation);
      }
   }
   
   //Make sure the piece is on the board
   void checkPosition() {
      if (rank > 7) {
         rank = 7;
      }
      if (rank < 0) {
         rank = 0;
      }
      if (file > 7) {
         file = 7;
      }
      if (file < 0) {
         file = 0;
      }
   }

   void print() {
      cout << "type: " << type << "\nrank: " << rank << "\nfile: " << file << "\nmaterial: " << material << "\nrotation: " << rotation << "\ndraw: " << draw << endl;
   }
};

//Helper class to represent the camera
class Camera {
public:
   //Position of the camera in space
   vec3 position;
   //Represent the horizontal angle
   float theta;
   //Represent the vertical angle
   float phi;

   Camera() {}

   Camera(vec3 pos, float t, float p) {
      position = pos;
      theta = t;
      phi = p;
   }

   //Set the position of the camera
   void setPosition(vec3 pos) {
      position = pos;
   }

   //Set horizontal angle
   void setTheta(float t) {
      theta = t;
   }

   //Set vertical angle
   void setPhi(float p) {
      phi = p;
   }

   //Return camera position
   vec3 getPosition() {
      return position;
   }

   //Return horizontal angle
   float getTheta() {
      return theta;
   }

   //Return vertical angle
   float getPhi() {
      return phi;
   }
};

#endif