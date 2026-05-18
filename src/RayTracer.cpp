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
#include <GL/gl.h>
#include <cmath>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 1500;
const int MAX_STEPS = 10;
const float XMIN = -17.77;
const float XMAX = 17.77;
const float YMIN = -10.0;
const float YMAX = 10.0;

vector<SceneObject *> sceneObjects;

TextureBMP texture;


// create the stripe pattern on a given object.
glm::vec3 stripe(Ray ray, glm::vec3 color, SceneObject* obj) {
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
  return color;
}

// calculate shadows on the specific point on the object that the ray has hit (taking
// into account the location of the lights)
glm::vec3 shadow(glm::vec3 color, SceneObject* obj, glm::vec3 lightPos, Ray ray) {
  color = obj->lighting(lightPos, -ray.dir, ray.hit); // Object's colour
  glm::vec3 lightVec = lightPos - ray.hit;
  Ray shadowRay(ray.hit, lightVec);
  shadowRay.closestPt(sceneObjects);
  shadowRay.dist = lightVec.length();
  
  if (shadowRay.index > -1) {
    color = (0.2f * 1 / shadowRay.dist) * obj->getColor();
  }
  return color;
}

//when a ray hits an object, create a reflection ray and return it
Ray createReflectionRay(SceneObject* obj, Ray ray) {
    glm::vec3 normalVec = obj->normal(ray.hit);
    glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
    Ray reflectedRay(ray.hit, reflectedDir);
    return reflectedRay;
}


//   Computes the colour value obtained by tracing a ray and finding its
//   closest point of intersection with objects in the scene.
glm::vec3 trace(Ray ray, int step) {
  glm::vec3 backgroundCol(0.2);     // Background colour = (0,0,0)
  glm::vec3 lightPos(10, 40, -3); // Light's position
  glm::vec3 color(0);
  SceneObject *obj;

  ray.closestPt(sceneObjects); // Compare the ray with all objects in the scene
  if (ray.index == -1)
    return backgroundCol;        // no intersection
  obj = sceneObjects[ray.index]; // object on which the closest point of intersection is found

  if (ray.index == 0) {
    color = stripe(ray, color, obj);
  }
  if (ray.index == 1) {
    color = glm::vec3(1,1,0);
    return color;
  }
  if (ray.index == 2) {
    color = glm::vec3(1,0,0);
    return color;
  }
  if (ray.index == 3) {
    color = glm::vec3(1,0,1);
    return color;
  }


  color = shadow(color, obj, lightPos, ray);

  if (obj->isReflective() && step < MAX_STEPS) {
    float rho = obj->getReflectionCoeff();
    Ray reflectedRay = createReflectionRay(obj, ray);
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

void createSphere(glm::vec3 color, glm::vec3 location, float radius, int specularity, vector<SceneObject *>* sceneObjects, float reflectivity) {
  Sphere *sphere1 = new Sphere(location, radius);
  sphere1->setColor(color); 
  sphere1->setSpecularity(specularity);
  if (reflectivity > 0) {
      sphere1->setReflectivity(true, reflectivity);
  } 
  else {
    sphere1->setReflectivity(false);
  }
  sceneObjects->push_back(sphere1); // Add sphere to scene objects
}

void createSkybox(glm::vec3 size, vector<SceneObject *>* sceneObjects) {
  Plane *backPlane = new Plane(glm::vec3(-size.x/2,-size.y/2,size.z),glm::vec3(size.x/2,-size.y/2,size.z),glm::vec3(size.x/2,size.y/2,size.z),glm::vec3(-size.x/2,size.y/2,size.z));
  sceneObjects->push_back(backPlane);
  Plane *leftPlane = new Plane(glm::vec3(-size.x/2,-size.y/2,-size.z),glm::vec3(-size.x/2,-size.y/2,size.z),glm::vec3(-size.x/2,size.y/2,size.z),glm::vec3(-size.x/2,size.y/2,-size.z));
  sceneObjects->push_back(leftPlane);
  Plane *rightPlane = new Plane(glm::vec3(size.x/2,-size.y/2,-size.z),glm::vec3(size.x/2,-size.y/2,size.z),glm::vec3(size.x/2,size.y/2,size.z),glm::vec3(size.x/2,size.y/2,-size.z));
  sceneObjects->push_back(rightPlane);
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
  texture = TextureBMP("../assets/Butterfly.bmp");
  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

  glClearColor(0, 0, 0, 1);

  Plane *plane = new Plane(glm::vec3(-50, -15, -40), glm::vec3(50, -15, -40),
                           glm::vec3(50, -15, -200), glm::vec3(-50, -15, -200));
  plane->setColor(glm::vec3(0.8, 0.8, 0));
  plane->setShininess(false);
  sceneObjects.push_back(plane);

  createSkybox(glm::vec3(100.0,100.0,-180.0), &sceneObjects);

  createSphere(glm::vec3(0, 0, 1), glm::vec3(-5.0, 0.0, -120.0), 15.0, 50, &sceneObjects, 0.5);

  createSphere(glm::vec3(0, 1, 1), glm::vec3(8, 7, -60.0), 5.0, 0, &sceneObjects, 0);

  createSphere(glm::vec3(0, 1, 0), glm::vec3(-8, 5, -70.0), 2.0, 0, &sceneObjects, 0);

  createSphere(glm::vec3(1, 0, 0), glm::vec3(15, -5, -70.0), 7.0, 0, &sceneObjects, 0);

}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(1280, 720);
  glutInitWindowPosition(20, 20);
  glutCreateWindow("Raytracing");

  glutDisplayFunc(display);
  initialize();

  glutMainLoop();
  return 0;
}
