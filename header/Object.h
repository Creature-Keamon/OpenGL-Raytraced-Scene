#ifndef OBJECT_H
#define OBJECT_H
#include "Cylinder.h"
#include "Sphere.h"
#include "Plane.h"
#include <vector>

Sphere *createSphere(glm::vec3 color, glm::vec3 location, float radius) {
  Sphere *sphere1 = new Sphere(location, radius);
  sphere1->setColor(color);

  return sphere1;
}

Cylinder *createCylinder(glm::vec3 color, glm::vec3 location, glm::vec3 size) {
  Cylinder *cylinder = new Cylinder(location, size);
  cylinder->setRadius();
  cylinder->setColor(color);

  return cylinder;
}

Plane *createMirror() {
  Plane *plane = new Plane(glm::vec3(-10, 5, -80), glm::vec3(10, 5, -80),
                           glm::vec3(10, 20, -75), glm::vec3(-10, 20, -77));
  //plane->setColor(glm::vec3(0.2, 0.2, 0.2));
  return plane;
}

void colourSphere(Sphere *sphere, int spec, float reflcoeff,
                  float refrcoeff = 1, float refrind = 0,
                  float transcoeff = 1) {
  sphere->setSpecularity(spec);
  if (reflcoeff > 0) {
    sphere->setReflectivity(true, reflcoeff);
  }
  if (refrcoeff < 1) {
    sphere->setRefractivity(true, refrcoeff, refrind);
  }
  if (transcoeff < 1) {
    sphere->setTransparency(true, transcoeff);
  }
}

void createSkybox(glm::vec3 size, std::vector<SceneObject *> *sceneObjects) {
  Plane *plane = new Plane(glm::vec3(-50, -5, 250), glm::vec3(50, -5, 250),
                           glm::vec3(50, -5, -250), glm::vec3(-50, -5, -250));
  plane->setColor(glm::vec3(0.8, 0.8, 0));
  plane->setReflectivity(false);
  sceneObjects->push_back(plane);
  Plane *backPlane =
      new Plane(glm::vec3(size.x / 2, -size.y / 2, -size.z / 2),
                glm::vec3(size.x / 2, size.y / 2, -size.z / 2),
                glm::vec3(-size.x / 2, size.y / 2, -size.z / 2),
                glm::vec3(-size.x / 2, -size.y / 2, -size.z / 2));
  backPlane->setColor(glm::vec3(0.2, 0.2, 0.2));
  backPlane->setReflectivity(false);
  sceneObjects->push_back(backPlane);
  Plane *leftPlane = new Plane(glm::vec3(-size.x / 2, -size.y / 2, size.z / 2),
                               glm::vec3(-size.x / 2, -size.y / 2, -size.z / 2),
                               glm::vec3(-size.x / 2, size.y / 2, -size.z / 2),
                               glm::vec3(-size.x / 2, size.y / 2, size.z / 2));
  leftPlane->setColor(glm::vec3(0, 1, 1));
  leftPlane->setReflectivity(false);
  sceneObjects->push_back(leftPlane);
  Plane *rightPlane = new Plane(glm::vec3(size.x / 2, -size.y / 2, -size.z / 2),
                                glm::vec3(size.x / 2, -size.y / 2, size.z / 2),
                                glm::vec3(size.x / 2, size.y / 2, size.z / 2),
                                glm::vec3(size.x / 2, size.y / 2, -size.z / 2));
  rightPlane->setColor(glm::vec3(1, 0, 0));
  rightPlane->setReflectivity(false);
  sceneObjects->push_back(rightPlane);

  Plane *behindPlane =
      new Plane(glm::vec3(-size.x / 2, -size.y / 2, size.z / 2),
                glm::vec3(size.x / 2, -size.y / 2, size.z / 2),
                glm::vec3(size.x / 2, size.y / 2, size.z / 2),
                glm::vec3(-size.x / 2, size.y / 2, size.z / 2));
  behindPlane->setColor(glm::vec3(1, 1, 0));
  behindPlane->setReflectivity(false);
  sceneObjects->push_back(behindPlane);

  Plane *roofPlane = new Plane(glm::vec3(-size.x / 2, size.y / 2, size.z * 2),
                               glm::vec3(size.x / 2, size.y / 2, size.z * 2),
                               glm::vec3(size.x / 2, size.y / 2, -size.z / 2),
                               glm::vec3(-size.x / 2, size.y / 2, -size.z / 2));
  roofPlane->setColor(glm::vec3(1, 0, 1));
  roofPlane->setReflectivity(false);
  sceneObjects->push_back(roofPlane);

  Plane *mirror = createMirror();
  mirror->setReflectivity(true);
  sceneObjects->push_back(mirror);
}

#endif