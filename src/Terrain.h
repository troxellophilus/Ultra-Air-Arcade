#pragma once
#ifndef __476__TERRAIN__
#define __476__TERRAIN__

#include <stdio.h>
#include "imageloader.h"
#include <vector>
#include <iostream>
#ifdef __APPLE__
#include <eigen3/Eigen/Dense>
#endif
#ifdef __unix__
#include <eigen3/Eigen/Dense>
#endif
#ifdef _WIN32
#define GLFW_INCLUDE_GLCOREARB
#include <GL/glew.h>
#include <cstdlib>
#include <stdlib.h>
#include <vector>
#endif
#include <string>

using namespace std;

class Terrain
{
   private:
      //VARIABLES
      int length;
      int width;
      
      //Find the normals after calculating heights
      void computeNormals();

   public:
      //Determine how large terrain will be. 
      double *heights;
      Eigen::Vector3f *normals;
      
      Terrain(const char* fileName, double baseHeight, vector<float>& posBuf, vector<unsigned int>& indBuf, vector<float>& norBuf);
      //Get the width of terrain.
      int getWidth(){return width;}
      //Get the length of the terrain.
      int getLength(){return length;}
      //Set the height of the terrain, may be made private.
      void setHeight(int w, int l, double h);
      //Load in image and then pass grayscale value to setHeight.
      void loadTerrain(string *fileName, double baseHeight);
		void createTerrain(vector<float>& posBuf, vector<unsigned int>& indBuf, vector<float>& norBuf);
      void loadTextures();
};

#endif
