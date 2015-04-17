#include "helper.h"

using namespace std;
//Given a vector of shapes which has already been read from an obj file
// resize all vertices to the range [-1, 1]
void loadShapes(const string &objFile, Object &obj) {
    string err = tinyobj::LoadObj(obj.shapes, obj.materials, objFile.c_str());
    if(!err.empty()) {
        cerr << err << endl;
    }
    resize_obj(obj.shapes);
    vector<float> norBuf;
    int idx1, idx2, idx3;
    glm::vec3 v1, v2, v3;
    //for every vertex initialize a normal to 0
    for (int j = 0; j < obj.shapes[0].mesh.positions.size()/3; j++) {
        norBuf.push_back(0);
        norBuf.push_back(0);
        norBuf.push_back(0);
    }
    // DO work here to compute the normals for every face
    //then add its normal to its associated vertex
    for (int i = 0; i < obj.shapes[0].mesh.indices.size()/3; i++) {
        idx1 = obj.shapes[0].mesh.indices[3*i+0];
        idx2 = obj.shapes[0].mesh.indices[3*i+1];
        idx3 = obj.shapes[0].mesh.indices[3*i+2];
        v1 = glm::vec3(obj.shapes[0].mesh.positions[3*idx1 +0],obj.shapes[0].mesh.positions[3*idx1 +1], obj.shapes[0].mesh.positions[3*idx1 +2]);
        v2 = glm::vec3(obj.shapes[0].mesh.positions[3*idx2 +0],obj.shapes[0].mesh.positions[3*idx2 +1], obj.shapes[0].mesh.positions[3*idx2 +2]);
        v3 = glm::vec3(obj.shapes[0].mesh.positions[3*idx3 +0],obj.shapes[0].mesh.positions[3*idx3 +1], obj.shapes[0].mesh.positions[3*idx3 +2]);
        
        // My code
        glm::vec3 u = v2 - v1;
        glm::vec3 v = v3 - v1;
        glm::vec3 n = glm::cross(u, v);
        n = glm::normalize(n); // Compute face normal
        
        //This is not correct, it sets the normal as the vertex value but
        //shows access pattern
        // Add face normal to each of its vertices
        norBuf[3*idx1+0] += n.x;
        norBuf[3*idx1+1] += n.y;
        norBuf[3*idx1+2] += n.z;
        
        norBuf[3*idx2+0] += n.x;
        norBuf[3*idx2+1] += n.y;
        norBuf[3*idx2+2] += n.z;
        
        norBuf[3*idx3+0] += n.x;
        norBuf[3*idx3+1] += n.y;
        norBuf[3*idx3+2] += n.z;
    }
    
    // Now normalize all of the vertex normals
    for (int i = 0; i < obj.shapes[0].mesh.positions.size()/3; i++) {
        glm::vec3 n = glm::vec3(norBuf[3*i + 0], norBuf[3*i + 1], norBuf[3*i + 2]);
        n = glm::normalize(n);
        norBuf[3*i + 0] = n.x;
        norBuf[3*i + 1] = n.y;
        norBuf[3*i + 2] = n.z;
    }
    
    obj.normals = norBuf;
}

void resize_obj(std::vector<tinyobj::shape_t> &shapes) {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    float scaleX, scaleY, scaleZ;
    float shiftX, shiftY, shiftZ;
    float epsilon = 0.001;
    
    minX = minY = minZ = 1.1754E+38F;
    maxX = maxY = maxZ = -1.1754E+38F;
    
    //Go through all vertices to determine min and max of each dimension
    for (size_t i = 0; i < shapes.size(); i++) {
        for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
            if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
            if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];
            
            if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
            if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];
            
            if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
            if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
        }
    }
    //From min and max compute necessary scale and shift for each dimension
    float maxExtent, xExtent, yExtent, zExtent;
    xExtent = maxX-minX;
    yExtent = maxY-minY;
    zExtent = maxZ-minZ;
    if (xExtent >= yExtent && xExtent >= zExtent) {
        maxExtent = xExtent;
    }
    if (yExtent >= xExtent && yExtent >= zExtent) {
        maxExtent = yExtent;
    }
    if (zExtent >= xExtent && zExtent >= yExtent) {
        maxExtent = zExtent;
    }
    scaleX = 2.0 /maxExtent;
    shiftX = minX + (xExtent/ 2.0);
    scaleY = 2.0 / maxExtent;
    shiftY = minY + (yExtent / 2.0);
    scaleZ = 2.0/ maxExtent;
    shiftZ = minZ + (zExtent)/2.0;
    
    //Go through all verticies shift and scale them
    for (size_t i = 0; i < shapes.size(); i++) {
        for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
            shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
            assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
            assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
            shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
            assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
            assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
            shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
            assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
            assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
        }
    }
}
