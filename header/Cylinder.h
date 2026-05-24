#ifndef H_CYLINDER
#define H_CYLINDER
#include "SceneObject.h"
#include <glm/glm.hpp>



class Cylinder : public SceneObject {
private:
    float R = 0;
    glm::vec3 location = glm::vec3(0);
    glm::vec3 size = glm::vec3(0,0,0);

public: 
    Cylinder(){};
    
    Cylinder(glm::vec3 l, glm::vec3 s): location(l), size(s) {};

    void setRadius();
    
    float intersect(glm::vec3 p0, glm::vec3 dir);

    glm::vec3 normal(glm::vec3 p);
};


#endif