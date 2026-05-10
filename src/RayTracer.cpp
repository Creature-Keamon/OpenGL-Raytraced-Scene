/*==================================================================================
 * COSC 363  Computer Graphics
 * Department of Computer Science and Software Engineering, University of
 * Canterbury.
 *
 * A basic ray tracer
 * See Lab07.pdf   for details.
 *===================================================================================
 */
#include "../header/Plane.h"
#include "../header/Ray.h"
#include "../header/SceneObject.h"
#include "../header/Sphere.h"
#include "../header/TextureBMP.h"
#include <GL/freeglut.h>
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;

vector<SceneObject *> sceneObjects;

TextureBMP texture;

//---The most important function in a ray tracer!
//----------------------------------
//   Computes the colour value obtained by tracing a ray and finding its
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step) {
  glm::vec3 backgroundCol(0);     // Background colour = (0,0,0)
  glm::vec3 lightPos(10, 40, -3); // Light's position
  glm::vec3 color(0);
  SceneObject *obj;

  ray.closestPt(sceneObjects); // Compare the ray with all objects in the scene
  if (ray.index == -1)
    return backgroundCol;        // no intersection
  obj = sceneObjects[ray.index]; // object on which the closest point of
                                 // intersection is found

  if (ray.index == 0) {
    // Stripe pattern
    int stripeWidth = 5;
    int iz = (ray.hit.z) / stripeWidth;
    int k = iz % 2; // 2 colors
    if (k == 0)
      color = glm::vec3(0, 1, 0);
    else
      color = glm::vec3(1, 1, 0.5);
    obj->setColor(color);

    float texcoords = (ray.hit.x - -15) / (5 - -15);
    float texcoordt = (ray.hit.z - -60) / (-90 - -60);
    if (texcoords > 0 && texcoords < 1 && texcoordt > 0 && texcoordt < 1) {
      color = texture.getColorAt(texcoords, texcoordt);
      obj->setColor(color);
    }
  }

  color = obj->lighting(lightPos, -ray.dir, ray.hit); // Object's colour
  glm::vec3 lightVec = lightPos - ray.hit;
  Ray shadowRay(ray.hit, lightVec);
  shadowRay.closestPt(sceneObjects);
  shadowRay.dist = lightVec.length();
  if (shadowRay.index > -1) {
    color = (0.2f * 1 / shadowRay.dist) * obj->getColor();
  }

  if (obj->isReflective() && step < MAX_STEPS) {
    float rho = obj->getReflectionCoeff();
    glm::vec3 normalVec = obj->normal(ray.hit);
    glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
    Ray reflectedRay(ray.hit, reflectedDir);
    glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
    color = color + (rho * reflectedColor);
  }

  return color;
}

//---The main display module
//-----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by
// drawing each cell as a quad.
//---------------------------------------------------------------------------------------
void display() {
  float xp, yp;                         // grid point
  float cellX = (XMAX - XMIN) / NUMDIV; // cell width
  float cellY = (YMAX - YMIN) / NUMDIV; // cell height
  glm::vec3 eye(0., 0., 0.);

  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glBegin(GL_QUADS); // Each cell is a tiny quad.

  for (int i = 0; i < NUMDIV; i++) { // Scan every cell of the image plane
    xp = XMIN + i * cellX;
    for (int j = 0; j < NUMDIV; j++) {
      yp = YMIN + j * cellY;

      glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY,
                    -EDIST); // direction of the primary ray

      Ray ray = Ray(eye, dir);

      glm::vec3 col =
          trace(ray, 1); // Trace the primary ray and get the colour value
      glColor3f(col.r, col.g, col.b);
      glVertex2f(xp, yp); // Draw each cell with its color value
      glVertex2f(xp + cellX, yp);
      glVertex2f(xp + cellX, yp + cellY);
      glVertex2f(xp, yp + cellY);
    }
  }

  glEnd();
  glFlush();
}

//---This function initializes the scene
//-------------------------------------------
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders
//   etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing
//   the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize() {
  texture = TextureBMP("../Butterfly.bmp");
  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

  glClearColor(0, 0, 0, 1);

  Plane *plane = new Plane(glm::vec3(-20, -15, -40), glm::vec3(20, -15, -40),
                           glm::vec3(20, -15, -200), glm::vec3(-20, -15, -200));
  plane->setColor(glm::vec3(0.8, 0.8, 0));
  plane->setShininess(false);
  sceneObjects.push_back(plane);
  Sphere *sphere1 = new Sphere(glm::vec3(-5.0, 0.0, -120.0), 15.0);
  sphere1->setColor(glm::vec3(0, 0, 1)); // Set colour to blue
  sphere1->setSpecularity(50);
  sphere1->setReflectivity(true, 0.8);
  sceneObjects.push_back(sphere1); // Add sphere to scene objects
  Sphere *sphere2 = new Sphere(glm::vec3(8, 7, -60.0), 5.0);
  sphere2->setColor(glm::vec3(0, 1, 1)); // Set colour to cyan
  sceneObjects.push_back(sphere2);       // Add sphere to scene objects
  Sphere *sphere3 = new Sphere(glm::vec3(-8, 5, -70.0), 1.0);
  sphere3->setColor(glm::vec3(0, 1, 0)); // Set colour to green
  sceneObjects.push_back(sphere3);       // Add sphere to scene objects
  Sphere *sphere4 = new Sphere(glm::vec3(15, -5, -70.0), 7.0);
  sphere4->setColor(glm::vec3(1, 0, 0)); // Set colour to red
  sceneObjects.push_back(sphere4);       // Add sphere to scene objects
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(20, 20);
  glutCreateWindow("Raytracing");

  glutDisplayFunc(display);
  initialize();

  glutMainLoop();
  return 0;
}
