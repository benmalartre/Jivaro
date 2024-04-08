#ifndef JVR_GEOMETRY_IMPLICIT_H
#define JVR_GEOMETRY_IMPLICIT_H

#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/capsule.h>
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

class Sphere : public Geometry {
public:
  Sphere();
  Sphere(const Sphere* other, bool normalize = true);
  Sphere(const pxr::UsdGeomSphere& sphere, const pxr::GfMatrix4d& world);
  virtual ~Sphere() {};

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

private:
  float                    _radius;  

};

class Plane : public Geometry {
public:
  Plane();
  Plane(const Plane* other, bool normalize = true);
  Plane(const pxr::UsdGeomPlane& plane, const pxr::GfMatrix4d& world);
  virtual ~Plane() {};

  pxr::GfVec3f GetNormal(bool worldSpace=true) {
    return worldSpace ? 
      _matrix.TransformDir(_normal) : 
      _normal;
  };

  pxr::GfVec3f GetOrigin(bool worldSpace=true) {
    return worldSpace ? 
      pxr::GfVec3f(_matrix[3][0], _matrix[3][1], _matrix[3][2]) : 
      pxr::GfVec3f(0.f);
  };

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

  void ComputeBoundingBox() override;

private:
  pxr::GfVec3f                _normal;
  float                       _width;
  float                       _length;
  bool                        _doubleSided;
};

class Cube : public Geometry {
public:
  Cube();
  Cube(const Cube* other, bool normalize = true);
  Cube(const pxr::UsdGeomCube& sphere, const pxr::GfMatrix4d& world);
  virtual ~Cube() {};

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

private:
  float                    _size;  

};

class Cone : public Geometry {
public:
  Cone();
  Cone(const Cone* other, bool normalize = true);
  Cone(const pxr::UsdGeomCone& sphere, const pxr::GfMatrix4d& world);
  virtual ~Cone() {};

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

private:
  float                    _radius;  
  float                    _height;
  pxr::TfToken             _axis;

};

class Capsule : public Geometry {
public:
  Capsule();
  Capsule(const Capsule* other, bool normalize = true);
  Capsule(const pxr::UsdGeomCapsule& sphere, const pxr::GfMatrix4d& world);
  virtual ~Capsule() {};

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

private:
  float                    _radius;  
  float                    _height;
  pxr::TfToken             _axis;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_IMPLICIT_H
