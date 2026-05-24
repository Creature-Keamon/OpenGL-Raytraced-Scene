/*--------------------------------------------------------------
* COSC363  Ray Tracer
*
*  The SceneObject class
*  This is a generic type for storing objects in the scene
*  Sphere, Plane etc. must be defined as subclasses of SceneObject.
*  Being an abstract class, this class cannot be instantiated.
-----------------------------------------------------------------*/

#include "../header/SceneObject.h"

glm::vec3 SceneObject::getColor() { return color_; }

glm::vec3 SceneObject::lighting(glm::vec3 lightPos, glm::vec3 viewVec,
                                glm::vec3 hit, glm::vec3 lightPos1 = glm::vec3(0)) {
  float ambientTerm = 0.2;
  float diffuseTerm = 0;
  float specularTerm = 0;
  float specularTerm1 = 0;
  glm::vec3 altSum = glm::vec3(0);
  
  glm::vec3 normalVec = normal(hit);
  glm::vec3 lightVec = lightPos - hit;
  lightVec = glm::normalize(lightVec);
  float lDotn = glm::dot(lightVec, normalVec);
  if (spec_) {
    glm::vec3 reflVec = glm::reflect(-lightVec, normalVec);
    float rDotv = glm::dot(reflVec, viewVec);
    if (rDotv > 0)
      specularTerm = pow(rDotv, shin_);
  }
  
  if (lightPos1 != glm::vec3(0)) {
    glm::vec3 lightVec1 = lightPos1 - hit;
    lightVec1 = glm::normalize(lightVec1);
    float lDotn1 = glm::dot(lightVec1, normalVec);
    if (spec_) {
      glm::vec3 reflVec1 = glm::reflect(-lightVec1, normalVec);
      float rDotv1 = glm::dot(reflVec1, viewVec);
      if (rDotv1 > 0)
        specularTerm1 = pow(rDotv1, shin_);
    }
    glm::vec3 diffuse1 = (lDotn1/3)* color_;
    glm::vec3 spec1 = (specularTerm1/3) * glm::vec3(1);
    altSum = (diffuse1) + spec1;
  }


  glm::vec3 ambientColor = (ambientTerm/3) * color_;
  glm::vec3 diffuse = (lDotn/3)* color_;
  glm::vec3 spec = (specularTerm/3) * glm::vec3(1);

  glm::vec3 colorSum = ambientColor + diffuse + spec + altSum;
  return colorSum;
}

float SceneObject::getReflectionCoeff() { return reflc_; }

float SceneObject::getRefractionCoeff() { return refrc_; }

float SceneObject::getTransparencyCoeff() { return tranc_; }

float SceneObject::getRefractiveIndex() { return refri_; }

float SceneObject::getShininess() { return shin_; }

bool SceneObject::isReflective() { return refl_; }

bool SceneObject::isRefractive() { return refr_; }

bool SceneObject::isSpecular() { return spec_; }

bool SceneObject::isTransparent() { return tran_; }

void SceneObject::setColor(glm::vec3 col) { color_ = col; }

void SceneObject::setReflectivity(bool flag) { refl_ = flag; }

void SceneObject::setReflectivity(bool flag, float refl_coeff) {
  refl_ = flag;
  reflc_ = refl_coeff;
}

void SceneObject::setRefractivity(bool flag) { refr_ = flag; }

void SceneObject::setRefractivity(bool flag, float refr_coeff,
                                  float refr_index) {
  refr_ = flag;
  refrc_ = refr_coeff;
  refri_ = refr_index;
}

void SceneObject::setShininess(float shininess) { shin_ = shininess; }

void SceneObject::setSpecularity(bool flag) { spec_ = flag; }

void SceneObject::setTransparency(bool flag) { tran_ = flag; }

void SceneObject::setTransparency(bool flag, float tran_coeff) {
  tran_ = flag;
  tranc_ = tran_coeff;
}
void SceneObject::setTransform(glm::mat4 transform) {
  transform = glm::inverse(transform);
  normalTransform = glm::transpose(transform);
  transformMatrix = transform;
}