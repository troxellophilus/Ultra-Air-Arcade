/*
 *  This system supports both physics based projectiles and spline based projectiles
 *  It can be used for all power ups (potentially) and possilbly assist the enemy AI.
 */

#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>
#include <vector>
#include "types.h"
#include "helper.h"

#define GLM_FORCE_RADIANS

#include "Entity.hpp"

using namespace std;

class Projectile{

private:
   Object obj;
   vector<Eigen::Vector3f> cps;
   vector<Eigen::Quaternionf> quats;   
   vector<pair<float,float> > usTable;
   bool drawSpline;   
   bool isSmart;
   bool isPath;
   glm::vec3 pos;
   glm::vec3 targetPos;
   Eigen::Vector3f position;
   Entity object;
   float distance;
   float tPrev;
   float t;
   float time;   

   void translateProjectile(Eigen::Vector3f trans);
   float s2u(float s);
   void buildTable();
   void updateProjectile(Eigen::Vector3f cp, Eigen::Matrix4f R);
   void updateCps(glm::vec3 pos, glm::vec3 targetPos, glm::vec3 initDir);
   void setCps(float x1, float x2, float x3, float x4);
   void launchPhysicsBased();
   int launchSplineBased(float elapsed, glm::vec3 myPos, glm::vec3 curPos, glm::vec3 initDir);

public:
   Projectile(Entity &e, bool path, bool smart, glm::vec3 initPos, glm::vec3 target);
   float getDistance(){return distance;}
   glm::vec3 startPos;
   int runProjectile(float elapsed, glm::vec3 myPos, glm::vec3 curPos, glm::vec3 initDir);
   Entity* getEntity(){return &object;}
};

Projectile::Projectile(Entity &e, bool path, bool smart, glm::vec3 initPos, glm::vec3 target) :
   t(0.0),
   tPrev(0.0)
{
   for(int i = 0; i < 3; i++){
      position[i] = initPos[i];
   }
   
   isPath = path;
   isSmart = smart;
   pos = initPos;
   startPos = initPos;
   targetPos = target;   

   object = e;
   
   distance = glm::distance(initPos, target);
   
   if(!path){
      time = (distance/5);
      cout << "Distance" << distance << endl;
      cout << "Time: " << time << endl;
      cout << "POS: " << pos[0] << ", " << pos[1] << ", " << pos[2] << endl;
      cout << "TARGET_POS: " << targetPos[0] << ", " << targetPos[1] << ", " << targetPos[2] << endl;
   }
   else{
      time = 10;
   }
   


   //cout << time << endl;
}

int Projectile::runProjectile(float elapsed, glm::vec3 myPos, glm::vec3 curPos, glm::vec3 initDir){
   if(isSmart){
      return launchSplineBased(elapsed, myPos, curPos, initDir);
   }
   else{
      launchPhysicsBased();
   }
   return 0;
}

void Projectile::updateCps(glm::vec3 pos, glm::vec3 targetPos, glm::vec3 initDir){
   if(!isPath){
      cps.clear();
  
      /*if(pos == targetPos){
         pos[0]+=.01;
         pos[1]+=.01;
         pos[2]+=.01;
      }*/
      //initDir = glm::normalize(initDir);      
 
      //cout << "startPos 2: "<< startPos[0] << ", " << startPos[1] << ", " << startPos[2] << endl; 
      setCps(startPos[0] - initDir[0] * 3, startPos[1] - initDir[1] * 3, startPos[2] - initDir[2] * 3, 1.0);   
      setCps(startPos[0] - initDir[0] * 3, startPos[1] - initDir[1] * 3, startPos[2] - initDir[2] * 3, 1.0);   
      setCps(startPos[0] - initDir[0] * 100, startPos[1], startPos[2] - initDir[2] * 100, 1.0);   
      //setCps(pos[0], pos[1] - initDir[1] * 3, pos[2], 1.0);   

      //cout << "CPS: " << startPos[0] + initDir[0] * 100 <<  ", " << startPos[1] << ", " << startPos[2] + initDir[2] * 100 << endl;
      
      for(int i = 0; i < 5; i++){
         setCps(targetPos[0], targetPos[1], targetPos[2], 1.0);
      } 

      //cout << "POS: " << pos[0] << ", " << pos[1] << ", " << pos[2] << endl;
      //cout << "MID: " << (pos[0] + targetPos[0])/2 << ", " << (pos[1] + targetPos[1])/2 << ", " << (pos[2] + targetPos[2])/2 << endl;     
      //cout << "END: " << (targetPos[0]) << ", " << (targetPos[1]) << ", " << (targetPos[2]) << endl;
   }
   else{
      cps.clear();
      setCps(startPos[0], startPos[1]+100.0, startPos[2]+100.0, 1.0);
      setCps(startPos[0], startPos[1]+100.0, startPos[2]+100.0, 1.0);
      setCps(pos[0]+100.0, pos[1]+100.0, pos[2]+100.0, 1.0);
      setCps(startPos[0]+50.0, startPos[1]+100.0, startPos[2], 1.0);
      setCps(startPos[0], startPos[1]+80.0, startPos[2]+100.0, 1.0);
      setCps(startPos[0], startPos[1]+100.0, startPos[2]+100.0, 1.0); 
   }
}

void Projectile::setCps(float x1, float x2, float x3, float x4){
   Eigen::Vector4f x;
   x(0) = x1;
   x(1) = x2;
   x(2) = x3;
   x(3) = x4;
   
   //cout << x[0] << ", " << x[1] << ", " << x[2] << endl;
   cps.push_back(x.segment<3>(0));
}

float Projectile::s2u(float s){
   float alpha;
   float prev;
   float u;
   float u0 = 0.0;

   prev = 0.0;
   for(int i = 1; i < (int)usTable.size(); i++) {
      pair<float,float> row = usTable[i];
      if(prev <= s && s <= row.second){
         alpha = (s - prev)/(row.second - prev);
         u = (1-alpha) * u0 + alpha * row.first;
         break;
      }
      prev = row.second;
      u0 = row.first;
   }
   return u;
}

void Projectile::buildTable(){
   usTable.clear();
   int ncps = (int)cps.size();
   if(ncps >= 4) {
      int numSegs = ncps - 3;
      float uTotal = 0;

      Eigen::MatrixXf G(3,4);
      Eigen::Vector3f curr;
      Eigen::Vector3f prev;
      Eigen::Vector4f uVec;
      Eigen::Vector4f uVecPrime;
      Eigen::Vector3f currPrime;
      Eigen::Vector3f prevPrime;

      float s;
      float sTotal = 0;
      float xi;
      float wi;
      usTable.push_back(make_pair(0, 0));
      Eigen::Matrix4f B;      
      B << 0, -1, 2, -1,
        2, 0, -5, 3,
        0, 1, 4, -3,
        0, 0, -1, 1;
   
      B /= 2;
      Eigen::Vector3f pPrime;
      for(int itr = 0; itr+3 < ncps; itr++){
         G << cps[itr](0), cps[itr+1](0), cps[itr+2](0), cps[itr+3](0),
              cps[itr](1), cps[itr+1](1), cps[itr+2](1), cps[itr+3](1),
              cps[itr](2), cps[itr+1](2), cps[itr+2](2), cps[itr+3](2);

         for(float u = 0.0; u <= 1; u+=.2){
            if(uTotal > 0 && u != 0){
               prev = curr;
               prevPrime = currPrime;

               for(int i = 1; i <= 3; i++){
                  if(i == 1){
                     wi = 5.0/9.0;
                     xi = -glm::sqrt(3.0/5.0);
                  }

                  if(i == 2){
                     wi = 8.0/9.0;
                     xi = 0;
                  }

                  if(i == 3){
                     wi = 5.0/9.0;
                     xi = glm::sqrt(3.0/5.0);
                  }
                  
                  float answer;

                  answer  = ((.2)/2.0) * xi + ((u + (u - .2))/2.0);

                  uVecPrime << 0, 1, 2 * answer, 3 * (answer * answer);
                  pPrime = G * B * uVecPrime;

                  sTotal += (.2/2.0) * wi * pPrime.norm();
               }
               usTable.push_back(make_pair(uTotal, sTotal));
               uTotal+=.2;
            }

            if(uTotal == 0)
               uTotal+=.2;
         }
      }
      
      Eigen::Vector3f tangent = pPrime.normalized();
   }
}

void Projectile::updateProjectile(Eigen::Vector3f cp, Eigen::Matrix4f R){
   //cout << "Position0: " << pos[0] << ", " << pos[1] << ", " << pos[2] << endl;
 
   pos[0] = cp[0]; 
   pos[1] = cp[1];
   pos[2] = cp[2];
   //position = pos + cp;
  
   //cout << "Position1: " << pos[0] << ", " << pos[1] << ", " << pos[2] << endl;
 
   object.setPosition(glm::vec3(pos[0], pos[1], pos[2])); 
   glm::mat4 rotation;
   for(int i = 0; i < 4; i++){
      for(int j = 0; j < 4; j++){
         rotation[i][j] = R(i,j);
      }
   }   
   if(!isPath){
      //cout << "R: " << R << endl;
   }
   //object.setTargetRotation(rotation);
}

void Projectile::launchPhysicsBased(){
   

}
int Projectile::launchSplineBased(float elapsed, glm::vec3 myPos, glm::vec3 targetPos, glm::vec3 initDir){
   Eigen::MatrixXf G(3,4);
   Eigen::Matrix4f B;
   Eigen::Vector4f uVec;

   t += (elapsed - tPrev);
   tPrev = elapsed;
    
   this->targetPos = targetPos;
 
   if(!isPath){
      //cout << "INIT DIR: " << initDir[0] << ", " << initDir[1] << ", " << initDir[2] << endl;
   }

   updateCps(myPos, targetPos,initDir);

   //alpha is the linear interpolation paramter between 0 and 2

   float alpha = std::fmod(t/100, time);

   //PRODUCE REAL TIME CONTROL POINTS
   glm::quat tempQuaternion = object.getRotationQ();   

   Eigen::Quaternionf q0, q1;

   //q0 = Eigen::AngleAxisf(90.0f/180.0f*M_PI, Eigen::Vector3f(0.0, 0.0, 0.0));

   //Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
   //R.block<3,3>(0,0) = q0.toRotationMatrix();

   Eigen::Vector3f p0, p1;
   p0 << -1.0f, 0.0f, 0.0f;
   p1 <<  1.0f, 0.0f, 0.0f;
   
   int ncps = (int)cps.size();

   //ROTATION ANGLE LOGIC SETUP WILL GO HERE LATER, WE WILL PUSH MORE INTO QUAT(QUATERNION)
   //quats.push_back(q0);  

   //glColor3f(1.0, 1.0, 1.0);
   
   //IMMEDIDIATE MODE IS DONE FOR DEBUGGING.
   //glBegin(GL_LINE_STRIP);
   //glLineWidth(1.5);      

   B << 0, -1, 2, -1,
        2, 0, -5, 3,
        0, 1, 4, -3,
        0, 0, -1, 1;
   
   B /= 2;
   
   //CREATE SPLINE FOR PROJECTILE TO FOLLOW
   for(int itr = 0; itr+3 < ncps; itr++){
      G << cps[itr](0), cps[itr+1](0), cps[itr+2](0), cps[itr+3](0),
           cps[itr](1), cps[itr+1](1), cps[itr+2](1), cps[itr+3](1),
           cps[itr](2), cps[itr+1](2), cps[itr+2](2), cps[itr+3](2);

      for(float u = 0.0; u < 1 && ncps > 3; u++){
         uVec << 1.0, u, u * u, u * u * u;
         Eigen::Vector3f p = G * B * uVec;   
      }
   }

   //HAVE PROJECTILE FOLLOW THE SPLINE
   buildTable();
   
   float sMax, s;
   pair<float,float> row = usTable[(int)usTable.size()-1];
   sMax = row.second;   
   float tNorm = std::fmod((double)t, (double)time) / time;

   float sNorm = tNorm;

   s = sMax * sNorm; 
      
   float kfloat;
   float u = s2u(s);
   //cout << "U: " << u << endl;
   
   int k = (int)std::floor(u);
   u = u - k;  
   
   //cout << "K: " << k << endl;
   G << cps[k](0), cps[k+1](0), cps[k+2](0), cps[k+3](0),   
        cps[k](1), cps[k+1](1), cps[k+2](1), cps[k+3](1),
        cps[k](2), cps[k+1](2), cps[k+2](2), cps[k+3](2);

   //ERROR ABOVE
   uVec << 1.0, u, u * u, u * u * u;
   Eigen::Vector3f p = G * B * uVec;
   Eigen::Vector4f uVecPrime;

   uVecPrime << 0, 1, 2 * u, 3 * (u * u);
   Eigen::Vector3f pPrime = G * B * uVecPrime;
   Eigen::Vector3f Tu = pPrime.normalized();   
   
   q0 = Eigen::AngleAxisf(M_PI, Tu);
   for(int i = 0; i < 4; i++){
      quats.push_back(q0);   
   }
   Eigen::Matrix4f gq;
      
   gq << quats[k].w(), quats[k+1].w(), quats[k+2].w(), quats[k+3].w(),   
         quats[k].x(), quats[k+1].x(), quats[k+2].x(), quats[k+3].x(),
         quats[k].y(), quats[k+1].y(), quats[k+2].y(), quats[k+3].y(),
         quats[k].z(), quats[k+1].z(), quats[k+2].z(), quats[k+3].z();        
   
   
   Eigen::Vector4f qVec = (gq * (B * uVec));
   Eigen::Quaternionf quaternion;
   quaternion.w() = qVec(0);
   quaternion.vec() = qVec.segment<3>(1);
   quaternion.normalize();

   Eigen::Matrix4f RLoc2 = Eigen::Matrix4f::Identity();
   RLoc2.block<3,3>(0,0) = quaternion.toRotationMatrix();
   
   
 
   //IMPLEMENT DRAW. IN OLD CODE I CALLED DRAW HELICOPTER HERE
   //cout << "INCLASS: " << p << endl;
   if(!isPath){
      //cout << "T: " << t << endl;
      //cout << "P: " << p[0] << ", " << p[1] << ", " << p[2] << endl;
      //cout << "K: " << k << endl;
      //cout << "U: " << u << endl;
      //cout << time << endl;
      //cout << p << endl;
   }

   if(p[0] == targetPos[0] && p[1] == targetPos[1] && p[2] == targetPos[2]){
      return 1;
   }
   
  
    
   updateProjectile(p, RLoc2);

   return 0;
}
#endif
