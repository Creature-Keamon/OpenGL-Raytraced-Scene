#include "../header/Cone.h"
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <math.h>

using namespace glm;

float Cone::intersect(vec3 p0, vec3 dir) {


}

vec3 Cone::normal(vec3 p) {
    float a = (180*atan((p.x-location.x)/(p.z-location.z)))/M_PI;
    float ang = (180*tan(R/size.y)/M_PI);
    vec3 n = vec3((180*(sin(a)*cos(ang)))/M_PI, (180*sin(ang))/M_PI, (180*(cos(a)*cos(ang)))/M_PI);
    n = normalize(n);
    return n;
}