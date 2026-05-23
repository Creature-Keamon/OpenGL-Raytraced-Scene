#ifndef H_CONE
#define H_CONE
#include "SceneObject.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

using namespace glm;

class Cone : public SceneObject {
    private:
    float R = 0;
    vec3 location = vec3(0);
    vec3 size = vec3(0);

    public: 
    Cone() {}
    Cone(glm::vec3 l, glm::vec3 s): location(l), size(s), R(sqrt(pow(s.x,2) + pow(s.z,2))) {}
    float intersect(vec3 p0, vec3 dir);

    vec3 normal(vec3 p);
};


#endif