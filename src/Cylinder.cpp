#include "../header/Cylinder.h"
#include <glm/glm.hpp>
#include <math.h>

void Cylinder::setRadius() {
    R = (sqrt(pow(size.x,2) + pow(size.z,2)));
}

float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir) {
  float a = dir.x * dir.x + dir.z * dir.z;
  float b = 2*(dir.x*(p0.x - location.x) + dir.z *(p0.z - location.z));
  float c = pow((p0.x - location.x),2)+ pow((p0.z - location.z),2) - (R * R);
  float delta = b * b - 4*a*c;
  if (delta < 0.001) 
    return -1.0;
  float t1 = (-b - sqrt(delta))/ (2*a);
  float t2 = (-b + sqrt(delta))/ (2*a);
  glm::vec3 p1 = p0 + t1*dir;
  glm::vec3 p2 = p0 + t2*dir;
  if (p1.y > location.y + size.y && p2.y <= location.y +size.y) {
    t1 = (location.y + size.y - p0.y)/dir.y;
  } 
  else if (p1.y > location.y + size.y && p2.y > location.y +size.y) {
    return -1;
  } 
  else if (p1.y < location.y && p2.y >= location.y +size.y) {
    t1 = (location.y + p0.y)/dir.y;
  }
  else if (p1.y < location.y && (p2.y < location.y )) {
    return -1;
  }
  if (t1 < 0) {
    return -1;
  } else
    return t1;
}

glm::vec3 Cylinder::normal(glm::vec3 p) {
    glm::vec3 n = glm::vec3((p.x-location.x),0,(p.z-location.z));
    n = normalize(n);
    return n;
}
