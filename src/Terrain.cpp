#include "Terrain.h"

/*Terrain::Terrain(int w, int l){   
   width = w;
   length = l;

   heights = new double[width][length];   
   normals = new Eigen::Vector3f[width][length];
}*/

void Terrain::setHeight(int w, int l, float h){


}

Terrain::Terrain(const char * fileName, float baseHeight, vector<float>& posBuf, vector<unsigned int>& indBuf, vector<float>& norBuf) {
   Image *image = loadBMP(fileName);
   
   this->width = image->width;
   this->length = image->height;

   //cout << width << length << endl;

   heights = new float[width * length];
   normals = new Eigen::Vector3f[width * length];
   
   int i =0;
   for(int z = 0; z < length; z++){
      for(int x = 0; x < width; x++){
         //cout << i << endl;
         unsigned char color = (unsigned char)image->pixels[3 * (z * image->width + x)];

         float h = baseHeight * (color/255.0);
         //cout << h << endl; 
         heights[z * width + x] = h;
         i++;
      }
   }
   
   computeNormals();
	createTerrain(posBuf, indBuf, norBuf);
   //delete image;
}

void Terrain::computeNormals(){
   Eigen::Vector3f sum;

   for(int y = 0; y < this->length; y++){
      for(int x = 0; x < this->width; x++){
         Eigen::Vector3f oVec;         
         Eigen::Vector3f iVec;
         Eigen::Vector3f rVec;
         Eigen::Vector3f lVec;

         sum << 0.0, 0.0, 0.0;
         
         //Add up all surrounding heights
         if(x > 0){
            lVec << -1.0, heights[y * width + (x-1)] - heights[y * width + x], 0.0;               
         }
         if(x < width - 1){
            rVec << 1.0, heights[y * width + (x+1)] - heights[y * width + x], 0.0;
         }
         if(y > 0){
            oVec << 0.0, heights[(y-1) * width + x] - heights[y * width + x], -1.0;
         }
         if(y < length - 1){
            iVec << 0.0,heights[(y+1) * width + x] - heights[y * width + x], 1.0;
         }      
      
         //Find vector which is perpendicular to oVec, lVec, rVec, and iVec
         if(x > 0 && y > 0){
            sum << sum + oVec.cross(lVec).normalized();
         }
         if(x > 0 && y < length - 1){
            sum << sum + lVec.cross(iVec).normalized();      
         }
         if(x < width - 1 && y < length - 1){
            sum << sum + iVec.cross(rVec).normalized();
         }
         if(x < width - 1 && y > 0){
            sum << sum + rVec.cross(oVec).normalized();   
         }         
            
         normals[y * width + x] = sum;
      }
   }
   
   for(int y = 0; y < length; y++){
      for(int x = 0; x < width; x++){   
         //Smooth out edges
         if(x > 0){
            sum << sum + normals[y * width + (x-1)] * .5;
         }
         if(x < width - 1){
            sum << sum + normals[y * width + (x+1)] * .5;
         }
         if(y > 0){
            sum << sum + normals[(y-1) * width + x] * .5;
         }
         if(y < length - 1){
            sum << sum + normals[(y+1) * width + x] * .5;
         }
         
         if(sum.norm() == 0){
            sum << 0.0, 1.0, 0.0;
         }
      }
   }            
}

void Terrain::createTerrain(vector<float>& posBuf, vector<unsigned int>& indBuf, vector<float>& norBuf){
   for(int z = 0; z < length; z++){
      for(int x = 0; x < width; x++){
         posBuf.push_back(x);
         posBuf.push_back(heights[z * width + x]);
         posBuf.push_back(z);
         
         norBuf.push_back(normals[z * width + x](0));
         norBuf.push_back(normals[z * width + x](1));
         norBuf.push_back(normals[z * width + x](2));
      }
   }  
   
   for(int x = 0; x < (width * length); x++){
      if((int)x % width != width - 1){
    
         indBuf.push_back(x + width + 1);
         indBuf.push_back(x + 1);
         indBuf.push_back(x);

         indBuf.push_back(x);
         indBuf.push_back(x + width);
         indBuf.push_back(x + width+1);

      }
   }
}

void Terrain::loadTextures(){
      




}


