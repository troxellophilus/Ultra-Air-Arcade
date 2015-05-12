#pragma once
#ifndef __476__TERRAIN__
#define __476__TERRAIN__

#include <stdio.h>
#include "ext/imageloader.h"
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
      float *heights;
      Eigen::Vector3f *normals;
      Terrain(const char* fileName, float baseHeight, vector<float>& posBuf, vector<unsigned int>& indBuf, vector<float>& norBuf);
      //Get the width of terrain.
      int getWidth(){return width;}
      //Get the length of the terrain.
      int getLength(){return length;}
      //Set the height of the terrain, may be made private.
      bool detectCollision(Eigen::Vector3f objVector, float radius);
      Eigen::Vector3f getNormal(Eigen::Vector3f objVector);
      void setHeight(int w, int l, float h);
      //Load in image and then pass grayscale value to setHeight.
      void loadTerrain(string *fileName, float baseHeight);
	void createTerrain(vector<float>& posBuf, vector<unsigned int>& indBuf, vector<float>& norBuf);
      void loadTextures();
};

#endif
