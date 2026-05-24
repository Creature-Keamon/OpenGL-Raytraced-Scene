#include "../header/Object.h"
#include "../header/Ray.h"
#include "../header/SceneObject.h"
#include "../header/Sphere.h"
#include "../header/stb_texture.h"
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <cmath>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <list>

using namespace std;
using namespace glm;

const bool aliasing = false;
const bool softShadows = false;
const float EDIST = 40.0;
const int NUMDIV = 600;
const int MAX_STEPS = 4;
const float XMIN = -10;
const float XMAX = 10;
const float YMIN = -10.0;
const float YMAX = 10.0;
vec3 lightPos1(-10, 10, -10);
float radius = 1;

vec3 spotLight(12, 20, -10);
float cutoff = 10;
vec3 direction(-1, -1.5, -1);

mat4 transform = mat4(1.0);
vector<SceneObject *> sceneObjects;

STBTexture texture;
STBTexture texture2;

// create the checker pattern on a given object.
vec3 checker(Ray ray, vec3 color, SceneObject *obj) {
  int checkerSize = 2;
  int iz = abs(ray.hit.z / checkerSize);
  int ix = abs(ray.hit.x / checkerSize);
  int k = iz % 2;
  int j = ix % 2;
  if (ray.hit.x < 0 || ray.hit.z > 0) {
    if ((k + j) % 2 == 1)
      color = vec3(1, 1, 1);
    else
      color = vec3(0.2, 0.2, 0.2);
  } else {
    if ((k + j) % 2 == 1)
      color = vec3(0.2, 0.2, 0.2);
    else
      color = vec3(1, 1, 1);
  }

  obj->setColor(color);
  return color;
}

vec2 createRandomVec(float radius) { return diskRand(radius); }

vec3 applyTexture(Ray ray, vec3 color, SceneObject *obj) {
  float x1 = -50, x2 = 50;
  float y1 = -15, y2 = 50;
  float texcoords = (ray.hit.x - x1) / (x2 - x1);
  float texcoordt = (ray.hit.y - y1) / (y2 - y1);
  if (texcoords > 0 && texcoords < 1 && texcoordt > 0 && texcoordt < 1) {
    color = texture.getColorAt(texcoords, 1 - texcoordt);
    obj->setColor(color);
  }
  return color;
}

vec3 applySphereTexture(Ray ray, vec3 color, SceneObject *obj) {
  vec3 ray2 = normalize(ray.hit - vec3(-4, 3, -60));
  float texcoords = (0.5 + (atan2(ray2.z, ray2.x)) / (2 * M_PI));
  float texcoordt = 1 - (0.5 + (asin(ray2.y) / M_PI));
  if (texcoords >= 0 && texcoords <= 1 && texcoordt >= 0 && texcoordt <= 1) {
    color = texture2.getColorAt(texcoords, 1 - texcoordt);
    obj->setColor(color);
  }
  return color;
}

vec3 shadowRayCalc(Ray ray, vec3 lightDir, vec3 lightPos, bool spot,
                   SceneObject *obj, vec3 color) {
  Ray shadowRay(ray.hit, lightDir);
  shadowRay.closestPt(sceneObjects);
  shadowRay.dist = lightDir.length();

  if (shadowRay.index > -1) {
    vec3 normDir = normalize(direction);
    vec3 shadowNorm = normalize(-shadowRay.dir);
    float dotProd = dot(shadowNorm, normDir);
    float angle = (180 * acos(dotProd)) / M_PI;

    float RQ = distance(shadowRay.hit, ray.hit);
    float LQ = distance(lightPos, ray.hit);

    if (RQ < LQ || spot && angle > cutoff) {
      SceneObject *hitObj = sceneObjects[shadowRay.index];

      if ((hitObj->isRefractive() || hitObj->isTransparent())) {
        vec3 ambient = 0.1f * obj->getColor();
        vec3 diffSpec = color - ambient;
        color = ambient + 0.5f * diffSpec;
        if (spot && angle > cutoff) {
          color = ambient;
        }
      } else {
        color = 0.1f * obj->getColor();
      }
    }
  }
  return color;
}

// calculate shadows on the specific point on the object that the ray has hit
// (taking into account the location of the lights)
vec3 shadow(SceneObject *obj, Ray ray, vec3 lightPos, bool tex = false,
            bool spot = false, bool area = false,
            vec3 lightPos1 = glm::vec3(0)) {
  vec3 color(0);
  vec3 refColor(0);
  vec3 color1(0);
  vec3 refColor1(0);
  if (tex) {
    color = obj->getColor();
    refColor = color;
  } else {
    color = obj->lighting(lightPos, -ray.dir, ray.hit,
                          lightPos1); // Object's colour
    refColor = color;
  }

  if (area) { // if the light source is an area light (soft shadows)
    list<vec3> colors;

    if (tex) {
      color1 = obj->getColor();
    } else {
      color1 = obj->lighting(lightPos, -ray.dir, ray.hit,
                             lightPos1); // Object's colour
    }
    refColor1 = color1;
    for (int i = 0; i < 15; i++) {
      if (tex) {
        color = obj->getColor();
        refColor = color;
      } else {
        color = obj->lighting(lightPos, -ray.dir, ray.hit,
                              lightPos1); // Object's colour
        refColor = color;
      }
      vec2 loc = createRandomVec(radius);
      vec3 lightDir = lightPos - ray.hit;
      color = shadowRayCalc(ray, lightDir, lightPos, spot, obj, color);

      if (lightPos1 != vec3(0)) {
        vec3 lightDir1 =
            vec3(lightPos1.x + loc.x, lightPos1.y, lightPos1.z + loc.y) -
            ray.hit;
        color1 = shadowRayCalc(ray, lightDir1, lightPos1, false, obj, color1);
        if (color1 != refColor1 && color != refColor) {
          color = 0.1f * obj->getColor();
        } else {
          color = color1 + color;
        }
      }
      colors.push_back(color);
    }
    color = vec3(0);
    for (vec3 newColor : colors) {
      color = color + newColor;
    }
    color /= colors.size();
  } else {
    vec3 lightDir = lightPos - ray.hit;
    color = shadowRayCalc(ray, lightDir, lightPos, spot, obj, color);
    if (lightPos1 != vec3(0)) {
      if (tex) {
        color1 = obj->getColor();
      } else {
        color1 = obj->lighting(lightPos, -ray.dir, ray.hit,
                               lightPos1); // Object's colour
      }
      refColor1 = color1;
      vec3 lightDir1 = lightPos1 - ray.hit;
      color1 = shadowRayCalc(ray, lightDir1, lightPos1, false, obj, color1);
      if (color1 != refColor1 && color != refColor) {
        color = 0.1f * obj->getColor();
      } else {
        color = color1 + color;
      }
    }
  }
  return color;
}

// when a ray hits an object, create a reflection ray and return it
Ray createReflectionRay(SceneObject *obj, Ray ray) {
  vec3 normalVec = obj->normal(ray.hit);
  vec3 reflectedDir = reflect(ray.dir, normalVec);
  Ray reflectedRay(ray.hit, reflectedDir);
  return reflectedRay;
}

//   Computes the colour value obtained by tracing a ray and finding its
//   closest point of intersection with objects in the scene.
vec3 trace(Ray ray, int step) {
  vec3 backgroundCol(0.2);
  vec3 color(0);
  vec3 color1(0);
  SceneObject *obj;

  ray.closestPt(sceneObjects); // Compare the ray with all objects in the scene
  if (ray.index == -1)
    return backgroundCol;
  obj = sceneObjects[ray.index];

  if (ray.index == 0) {
    color = checker(ray, color, obj);
  }

  if (ray.index == 1) {
    color = applyTexture(ray, color, obj);
    color = shadow(obj, ray, spotLight, true, true, softShadows, lightPos1);
    return color;
  }

  if (ray.index == 6 && step < MAX_STEPS) {
    Ray reflectedRay = createReflectionRay(obj, ray);
    vec3 reflectedColor = trace(reflectedRay, step + 1);
    return reflectedColor;
  }

  if (ray.index == 10 && step < MAX_STEPS) {
    color = applySphereTexture(ray, color, obj);
    color = shadow(obj, ray, spotLight, true, true, softShadows, lightPos1);
    return color;
  }

  color = shadow(obj, ray, spotLight, false, true, softShadows, lightPos1);

  // refractions
  if (obj->isRefractive() && step < MAX_STEPS) {
    float eta2 = obj->getRefractiveIndex();
    float refr = obj->getRefractionCoeff();
    vec3 n = obj->normal(ray.hit);
    vec3 d = ray.dir;
    vec3 g = refract(d, n, 1 / eta2);
    Ray refrRay = Ray(ray.hit, g);
    refrRay.closestPt(sceneObjects);

    vec3 m = obj->normal(refrRay.hit);
    vec3 h = refract(g, -m, 1 / eta2);
    Ray newRay = Ray(refrRay.hit, h);
    newRay.closestPt(sceneObjects);
    vec3 refractColor = trace(newRay, step + 1);

    color = color + (refr * refractColor);
  }

  // transparency
  if (obj->isTransparent() && step < MAX_STEPS) {
    float trans = obj->getTransparencyCoeff();
    vec3 n = obj->normal(ray.hit);
    vec3 d = ray.dir;
    Ray refrRay = Ray(ray.hit, d);
    refrRay.closestPt(sceneObjects);

    Ray newRay = Ray(refrRay.hit, d);
    newRay.closestPt(sceneObjects);
    vec3 transparentColor = trace(newRay, step + 1);
    color = color + (trans * transparentColor);
  }

  // reflections
  if (obj->isReflective() && step < MAX_STEPS) {
    float rho = obj->getReflectionCoeff();
    Ray reflectedRay = createReflectionRay(obj, ray);
    vec3 reflectedColor = trace(reflectedRay, step + 1);
    color = color + (rho * reflectedColor);
  }

  return color;
}

void display() {
  float xp, yp;                         // grid point
  float cellX = (XMAX - XMIN) / NUMDIV; // cell width
  float cellY = (YMAX - YMIN) / NUMDIV; // cell height
  vec3 eye(0., 0., 0.);

  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glBegin(GL_QUADS); // Each cell is a tiny quad.

  for (int x = 0; x < NUMDIV; x++) { // Scan every cell of the image plane
    xp = XMIN + (x * cellX);
    for (int y = 0; y < NUMDIV; y++) {
      yp = YMIN + (y * cellY);

      if (aliasing) {

        vec3 dir0(xp + 0.75 * cellX, yp + 0.75 * cellY,
                  -EDIST); // direction of the top left ray
        Ray ray0 = Ray(eye, dir0);
        vec3 col0 = trace(ray0, 1);

        vec3 dir1(xp + 0.75 * cellX, yp + 0.25 * cellY,
                  -EDIST); // direction of the bottom left ray
        Ray ray1 = Ray(eye, dir1);
        vec3 col1 = trace(ray1, 1);

        vec3 dir2(xp + 0.25 * cellX, yp + 0.25 * cellY,
                  -EDIST); // direction of the bottom right ray
        Ray ray2 = Ray(eye, dir2);
        vec3 col2 = trace(ray2, 1);

        vec3 dir3(xp + 0.25 * cellX, yp + 0.75 * cellY,
                  -EDIST); // direction of the top right ray
        Ray ray3 = Ray(eye, dir3);
        vec3 col3 = trace(ray3, 1);

        vec3 average((col0.r + col1.r + col2.r + col3.r) / 4,
                     (col0.g + col1.g + col2.g + col3.g) / 4,
                     (col0.b + col1.b + col2.b + col3.b) / 4);

        glColor3f(average.x, average.g, average.b);

      }

      else {
        vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);
        Ray ray = Ray(eye, dir);
        vec3 col = trace(ray, 1);
        glColor3f(col.x, col.y, col.z);
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

void initialize() {
  texture = STBTexture("../assets/FlowerField.jpg");
  texture2 = STBTexture("../assets/fuggler.jpg");
  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

  glClearColor(0, 0, 0, 1);

  createSkybox(vec3(100.0, 100.0, 500.0), &sceneObjects);

  // transformed sphere (pill shaped thing)
  Sphere *sphere1 = createSphere(vec3(0, 0, 1), vec3(0, -1.5, -50), 3);
  colourSphere(sphere1, 50, 0.1);
  mat4 transform = mat4(1.0);
  transform = scale(transform, vec3(0.5, 1.0, 0.5));
  sphere1->setTransform(transform);
  sceneObjects.push_back(sphere1);

  // refractive sphere
  Sphere *sphere2 = createSphere(vec3(0, 0.1, 0), vec3(-8, -1.4, -40), 4.0);
  colourSphere(sphere2, 50, 0.1, 0.6, 1.1);
  sceneObjects.push_back(sphere2);

  // transparent sphere
  Sphere *sphere3 = createSphere(vec3(0.1, 0, 0), vec3(8, -1.4, -40), 4.0);
  colourSphere(sphere3, 50, 0.1, 1, 1.01, 0.5);
  sceneObjects.push_back(sphere3);

  // textured sphere
  Sphere *sphere4 = createSphere(vec3(1, 1, 1), vec3(-4, 3, -60), 4.0);
  sceneObjects.push_back(sphere4);

  // cylinder
  Cylinder *cylinder =
      createCylinder(vec3(1, 0, 0.3), vec3(4, 3, -65), vec3(2, 5, 2));
  sceneObjects.push_back(cylinder);
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(600, 600);
  glutInitWindowPosition(20, 20);
  glutCreateWindow("Raytracing");
  glutDisplayFunc(display);
  initialize();

  glutMainLoop();
  return 0;
}
