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
#include <vector>

using namespace std;

const bool aliasing = false;
const float EDIST = 40.0;
const int NUMDIV = 800;
const int MAX_STEPS = 10;
const float XMIN = -17.77;
const float XMAX = 17.77;
const float YMIN = -10.0;
const float YMAX = 10.0;
glm::vec3 lightPos(20, 10, 30);

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
glm::vec3 shadow(SceneObject* obj, Ray ray) {
  glm::vec3 color = obj->lighting(lightPos, -ray.dir, ray.hit); // Object's colour
  glm::vec3 lightDir = lightPos - ray.hit;
  glm::vec3 normLight = glm::normalize(lightDir);
  Ray shadowRay(ray.hit, normLight);
  shadowRay.closestPt(sceneObjects);
  shadowRay.dist = normLight.length();
  
  float RQ = glm::distance(shadowRay.hit,ray.hit);
  float LQ = glm::distance(lightPos, ray.hit);
  
  if (RQ < LQ) {
    if (shadowRay.index > -1) {
      SceneObject* hitObj = sceneObjects[shadowRay.index];
      if (hitObj->isRefractive() || hitObj->isTransparent()) {
        glm::vec3 ambient = 0.2f * obj->getColor();
        glm::vec3 diffSpec = color - ambient;
        color = ambient + 0.7f*diffSpec;
      }
      else {
        color = (0.2f * obj->getColor());
      }   
    }
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
  glm::vec3 backgroundCol(0.2); 
  glm::vec3 color(0);
  SceneObject *obj;

  ray.closestPt(sceneObjects); // Compare the ray with all objects in the scene
  if (ray.index == -1)
    return backgroundCol;        // no intersection
  obj = sceneObjects[ray.index]; // object on which the closest point of intersection is found

  if (ray.index == 0) {
    color = stripe(ray, color, obj);
  }

  color = shadow(obj, ray);

  if (obj->isRefractive() && step < MAX_STEPS) {
    float eta2 = obj->getRefractiveIndex();
    float refr = obj->getRefractionCoeff();
    glm::vec3 n = obj->normal(ray.hit);
    glm::vec3 d = ray.dir;
    glm::vec3 g = glm::refract(d, n, 1/eta2);
    Ray refrRay = Ray(ray.hit, g);
    refrRay.closestPt(sceneObjects);
    
    glm::vec3 m = obj->normal(refrRay.hit);
    glm::vec3 h = glm::refract(g,-m, 1/eta2);
    Ray newRay = Ray(refrRay.hit, h);
    newRay.closestPt(sceneObjects);
    glm::vec3 refractColor = trace(newRay, step + 1);
    
    color = color + (refr * refractColor);
  }

  if (obj->isTransparent() && step <MAX_STEPS) {
    float trans = obj->getTransparencyCoeff();
    glm::vec3 n = obj->normal(ray.hit);
    glm::vec3 d = ray.dir;
    Ray refrRay = Ray(ray.hit, d);
    refrRay.closestPt(sceneObjects);
    
    Ray newRay = Ray(refrRay.hit, d);
    newRay.closestPt(sceneObjects);
    glm::vec3 transparentColor = trace(newRay, step + 1);
    color = color + (trans * transparentColor);
  }

  if (obj->isReflective() && step < MAX_STEPS) {
    float rho = obj->getReflectionCoeff();
    Ray reflectedRay = createReflectionRay(obj, ray);
    glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
    color = color + (rho * reflectedColor);
  }

  return color;
}


void display() {
  float xp, yp;                         // grid point
  glm::vec3 prevcolor(0,0,0);
  float cellX = (XMAX - XMIN) / NUMDIV; // cell width
  float cellY = (YMAX - YMIN) / NUMDIV; // cell height
  glm::vec3 eye(0., 0., 0.);

  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glBegin(GL_QUADS); // Each cell is a tiny quad.

  for (int x = 0; x < NUMDIV; x++) { // Scan every cell of the image plane
    xp = XMIN + (x * cellX);
    for (int y = 0; y < NUMDIV; y++) {
      yp = YMIN + (y * cellY);
      glm::vec3 dir(xp+ 0.5 * cellX, yp + 0.5 * cellY, -EDIST);
      Ray ray = Ray(eye,dir);
      glm::vec3 col = trace(ray, 1);
      if (aliasing) {
        if (col.x == prevcolor.x && col.y == prevcolor.y && col.z == prevcolor.z) {
        glColor3f(col.x,col.y,col.z);
        } 
        else {
          glm::vec3 dir0(xp + 0.75 * cellX, yp + 0.75 * cellY,
                      -EDIST); // direction of the top left ray
          Ray ray0 = Ray(eye, dir0);
          glm::vec3 col0 = trace(ray0, 1);
          
          glm::vec3 dir1(xp + 0.75 * cellX, yp + 0.25 * cellY,
                        -EDIST); // direction of the bottom left ray
          Ray ray1 = Ray(eye, dir1);
          glm::vec3 col1 = trace(ray1, 1);
          
          glm::vec3 dir2(xp + 0.25 * cellX, yp + 0.25 * cellY,
                        -EDIST); // direction of the bottom right ray
          Ray ray2 = Ray(eye, dir2);
          glm::vec3 col2 = trace(ray2, 1);
          
          glm::vec3 dir3(xp + 0.25 * cellX, yp + 0.75 * cellY,
                        -EDIST); // direction of the top right ray
          Ray ray3 = Ray(eye, dir3);
          glm::vec3 col3 = trace(ray3, 1);

          glm::vec3 average((col0.r + col1.r + col2.r + col3.r)/4, 
                            (col0.g + col1.g + col2.g + col3.g)/4, 
                            (col0.b + col1.b + col2.b + col3.b)/4);

          prevcolor = average;
          glColor3f(average.x, average.g,average.b);   
        }
      }
      else {
        glColor3f(col.x,col.y,col.z);
      }
      
      
      glVertex2f(xp, yp); // Draw each cell with its color value
      glVertex2f(xp + cellX, yp);
      glVertex2f(xp + cellX, yp + cellY);
      glVertex2f(xp, yp + cellY);
    }
  }

  glEnd();
  glFlush();
}

Sphere* createSphere(glm::vec3 color, glm::vec3 location, float radius) {
  Sphere *sphere1 = new Sphere(location, radius);
  sphere1->setColor(color); 

  return sphere1;
}

void colourSphere(Sphere *sphere, int spec, float reflcoeff, float refrcoeff = 1, float refrind = 0, float transcoeff = 1) {
  sphere->setSpecularity(spec);
  if (reflcoeff > 0) {
      sphere->setReflectivity(true, reflcoeff);
  } 
  if (refrcoeff < 1) {
    sphere->setRefractivity(true, refrcoeff, refrind);
  }
  if (transcoeff <1) {
    sphere->setTransparency(true, transcoeff);
  }
}

void createSkybox(glm::vec3 size, vector<SceneObject *>* sceneObjects) {
  Plane *plane = new Plane(glm::vec3(-50, -15, 250), 
                           glm::vec3(50, -15, 250),
                           glm::vec3(50, -15, -250), 
                           glm::vec3(-50, -15, -250));
  plane->setColor(glm::vec3(0.8, 0.8, 0));
  plane->setShininess(false);
  sceneObjects->push_back(plane);
  Plane *backPlane = new Plane(glm::vec3(-size.x/2,-size.y/2,size.z/2),
                               glm::vec3(size.x/2,-size.y/2,size.z/2),
                               glm::vec3(size.x/2,size.y/2,size.z/2),
                               glm::vec3(-size.x/2,size.y/2,size.z/2));
  backPlane->setColor(glm::vec3(1,1,0));
  sceneObjects->push_back(backPlane);
  Plane *leftPlane = new Plane(glm::vec3(-size.x/2,-size.y/2,size.z/2),
                               glm::vec3(-size.x/2,-size.y/2,-size.z/2),
                               glm::vec3(-size.x/2,size.y/2,-size.z/2),
                               glm::vec3(-size.x/2,size.y/2,size.z/2));
  leftPlane->setColor(glm::vec3(0,1,1));                           
  sceneObjects->push_back(leftPlane);
  Plane *rightPlane = new Plane(glm::vec3(size.x/2,-size.y/2,-size.z/2),
                                glm::vec3(size.x/2,-size.y/2,size.z/2),
                                glm::vec3(size.x/2,size.y/2,size.z/2),
                                glm::vec3(size.x/2,size.y/2,-size.z/2));
  rightPlane->setColor(glm::vec3(1,0,0));                              
  sceneObjects->push_back(rightPlane);
  Plane *behindPlane = new Plane(glm::vec3(-size.x/2,-size.y/2,-size.z/2),
                                 glm::vec3(size.x/2,-size.y/2,-size.z/2),
                                 glm::vec3(size.x/2,size.y/2,-size.z/2),
                                 glm::vec3(-size.x/2,size.y/2,-size.z/2));
  behindPlane->setColor(glm::vec3(1,0,1));                                 
  sceneObjects->push_back(behindPlane);
  
  Plane *roofPlane = new Plane(glm::vec3(-size.x/2,size.y/2,size.z*2),
                                glm::vec3(size.x/2,size.y/2,size.z*2),
                                 glm::vec3(size.x/2,size.y/2,-size.z/2),
                                 glm::vec3(-size.x/2,size.y/2,-size.z/2));
  roofPlane->setColor(glm::vec3(0,0,1));                                   
  sceneObjects->push_back(roofPlane);

}


void initialize() {
  texture = TextureBMP("../assets/Butterfly.bmp");
  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

  glClearColor(0, 0, 0, 1);


  createSkybox(glm::vec3(100.0,100.0,500.0), &sceneObjects);
  
  Sphere * sphere1 = createSphere(glm::vec3(0, 0, 1), 
            glm::vec3(-5, 0, -120), 15.0);
  colourSphere(sphere1, 50, 0.1);
  sceneObjects.push_back(sphere1);

  Sphere *sphere2 = createSphere(glm::vec3(0, 0.1, 0), 
            glm::vec3(-12, 0, -50), 5.0);
  colourSphere(sphere2, 50, 0.1, 0.6, 1.1);
  sceneObjects.push_back(sphere2);


  Sphere * sphere3 = createSphere(glm::vec3(0.1, 0, 0), 
            glm::vec3(10, 0, -50), 5.0);
  colourSphere(sphere3, 50, 0.1,1,1.01, 0.5);
  sceneObjects.push_back(sphere3);

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
