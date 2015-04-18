#include "Object.h"

Object::Object() {

}

Object::Object(vec3 pos, vec3 dir, float vel, AABB bb) {
   position = pos;
   direction = dir;
   velocity = vel;
   boundingBox = bb;

   touched = false;
   posBufObj = 0;
   norBufObj = 0;
   indBufObj = 0;
}

Object::~Object() {
   // TODO Auto-generated destructor stub
}

//Given a vector of shapes which has already been read from an obj file
// resize all vertices to the range [-1, 1]
void Object::resize_obj() {
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t i = 0; i < shape.size(); i++) {
      for (size_t v = 0; v < shape[i].mesh.positions.size() / 3; v++) {
         if (shape[i].mesh.positions[3 * v + 0] < minX) minX = shape[i].mesh.positions[3 * v + 0];
         if (shape[i].mesh.positions[3 * v + 0] > maxX) maxX = shape[i].mesh.positions[3 * v + 0];

         if (shape[i].mesh.positions[3 * v + 1] < minY) minY = shape[i].mesh.positions[3 * v + 1];
         if (shape[i].mesh.positions[3 * v + 1] > maxY) maxY = shape[i].mesh.positions[3 * v + 1];

         if (shape[i].mesh.positions[3 * v + 2] < minZ) minZ = shape[i].mesh.positions[3 * v + 2];
         if (shape[i].mesh.positions[3 * v + 2] > maxZ) maxZ = shape[i].mesh.positions[3 * v + 2];
      }

      //From min and max compute necessary scale and shift for each dimension
      float maxExtent, xExtent, yExtent, zExtent;
      xExtent = maxX - minX;
      yExtent = maxY - minY;
      zExtent = maxZ - minZ;
      if (xExtent >= yExtent && xExtent >= zExtent) {
         maxExtent = xExtent;
      }
      if (yExtent >= xExtent && yExtent >= zExtent) {
         maxExtent = yExtent;
      }
      if (zExtent >= xExtent && zExtent >= yExtent) {
         maxExtent = zExtent;
      }
      scaleX = 2.0 / maxExtent;
      shiftX = minX + (xExtent / 2.0);
      scaleY = 2.0 / maxExtent;
      shiftY = minY + (yExtent / 2.0);
      scaleZ = 2.0 / maxExtent;
      shiftZ = minZ + (zExtent) / 2.0;

      //Go through all verticies shift and scale them
      for (size_t v = 0; v < shape[i].mesh.positions.size() / 3; v++) {
         shape[i].mesh.positions[3 * v + 0] = (shape[i].mesh.positions[3 * v + 0] - shiftX) * scaleX;
         assert(shape[i].mesh.positions[3 * v + 0] >= -1.0 - epsilon);
         assert(shape[i].mesh.positions[3 * v + 0] <= 1.0 + epsilon);
         shape[i].mesh.positions[3 * v + 1] = (shape[i].mesh.positions[3 * v + 1] - shiftY) * scaleY;
         assert(shape[i].mesh.positions[3 * v + 1] >= -1.0 - epsilon);
         assert(shape[i].mesh.positions[3 * v + 1] <= 1.0 + epsilon);
         shape[i].mesh.positions[3 * v + 2] = (shape[i].mesh.positions[3 * v + 2] - shiftZ) * scaleZ;
         assert(shape[i].mesh.positions[3 * v + 2] >= -1.0 - epsilon);
         assert(shape[i].mesh.positions[3 * v + 2] <= 1.0 + epsilon);
      }
   }
}

void Object::loadShapes(const string &objFile) {
   string err = tinyobj::LoadObj(shape, material, objFile.c_str());
   if (!err.empty()) {
      cerr << err << endl;
   }
   resize_obj();
}

vector<float> Object::getNorBuf() {
   vector<float> norBuf;
   int idx1, idx2, idx3;
   vec3 v1, v2, v3, normal;
   //for every vertex initialize a normal to 0
   for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
      norBuf.push_back(0);
      norBuf.push_back(0);
      norBuf.push_back(0);
   }
   //compute the normals for every face
   //then add its normal to its associated vertex
   for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
      idx1 = shape[0].mesh.indices[3 * i + 0];
      idx2 = shape[0].mesh.indices[3 * i + 1];
      idx3 = shape[0].mesh.indices[3 * i + 2];
      v1 = vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
      v2 = vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
      v3 = vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);

      normal = normalize(cross(v2 - v1, v3 - v1));

      norBuf[3 * idx1 + 0] += normal.x;
      norBuf[3 * idx1 + 1] += normal.y;
      norBuf[3 * idx1 + 2] += normal.z;
      norBuf[3 * idx2 + 0] += normal.x;
      norBuf[3 * idx2 + 1] += normal.y;
      norBuf[3 * idx2 + 2] += normal.z;
      norBuf[3 * idx3 + 0] += normal.x;
      norBuf[3 * idx3 + 1] += normal.y;
      norBuf[3 * idx3 + 2] += normal.z;
   }

   return norBuf;
}

//Initialize the buffers for each loaded shape
void Object::initShape() {
   // Send the position array to the GPU
   const vector<float> &posBuf = shape[0].mesh.positions;
   glGenBuffers(1, &posBufObj);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObj);
   glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

   // compute the normals per vertex 
   vector<float> norBuf = getNorBuf();

   glGenBuffers(1, &norBufObj);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObj);
   glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

   // Send the index array to the GPU
   const vector<unsigned int> &indBuf = shape[0].mesh.indices;
   glGenBuffers(1, &indBufObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

   // Unbind the arrays
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   GLSL::checkVersion();
   assert(glGetError() == GL_NO_ERROR);
}


void Object::timeStep(float dt) {
   /*
   The step function receives one parameter, dt, the elapsed time.  It must update the position by 
converting dt into elapsed time in seconds, then using that time value, the velocity and direction, 
update the position. However, it must check two constraints: 
1.
If the new position would be off the grid, negate the direction vector and recompute the new 
position. 
2.
If the new position would cause the object's bounding box to intersect the bounding of any 
other object, do not update the position. 
   */

   if (!touched) {
      //position = position + direction * velocity * dt;

      position = position + direction * velocity;
      
      if (position.x > 48 || position.x < -48 || position.z > 48 || position.z < -48) {
         direction = -direction;
      }

      boundingBox = AABB(position, vec3(0.9, 0.9, 0.9));
   }
}

void Object::draw(GLint h_aPosition, GLint h_aNormal, GLint h_uModelMatrix, RenderingHelper ModelTrans) {
   GLSL::enableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBufObj);
   glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Enable and bind normal array for drawing
   GLSL::enableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, norBufObj);
   glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Bind index array for drawing
   int nIndices = (int)shape[0].mesh.indices.size();
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObj);

   glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
   glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}

