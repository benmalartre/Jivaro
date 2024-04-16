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
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/capsule.h>
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

class Xform : public Geometry {
public:
  Xform(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Xform(const Xform* other);
  Xform(const pxr::UsdGeomXform& xform, const pxr::GfMatrix4d& world);
  virtual ~Xform() {};
};

class Sphere : public Geometry {
public:
  Sphere(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Sphere(const Sphere* other, bool normalize = true);
  Sphere(const pxr::UsdGeomSphere& sphere, const pxr::GfMatrix4d& world);
  virtual ~Sphere() {};

  void SetRadius(float radius){_radius = radius;};
  float GetRadius() {return _radius;};
  pxr::GfVec3f GetCenter(){return pxr::GfVec3f(GetMatrix().GetRow3(3));};

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
  Plane(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
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

private:
  pxr::GfVec3f                _normal;
  float                       _width;
  float                       _length;
  bool                        _doubleSided;
};

class Cube : public Geometry {
public:
  Cube(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
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
  Cone(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
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
  Capsule(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
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
