#ifndef JVR_GEOMETRY_IMPLICIT_H
#define JVR_GEOMETRY_IMPLICIT_H

#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/capsule.h>

#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

class Xform : public Geometry {
public:
  Xform(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Xform(const pxr::UsdGeomXform& xform, const pxr::GfMatrix4d& world);
  virtual ~Xform() {};

  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override{};

};

class Plane : public Geometry {
public:
  Plane(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Plane(const pxr::UsdGeomPlane& plane, const pxr::GfMatrix4d& world);
  virtual ~Plane() {};


  pxr::GfVec3f GetNormal();
  pxr::GfVec3f GetOrigin();

  pxr::GfPlane Get();

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  float SignedDistance(const pxr::GfVec3f& point) const override;

protected:
  Geometry::DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;

  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& time=pxr::UsdTimeCode::Default()) override;

private:
  pxr::TfToken                _axis;
  pxr::GfVec3f                _normal;
  double                      _width;
  double                      _length;
  bool                        _doubleSided;
};

class Sphere : public Geometry {
public:
  Sphere(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Sphere(const pxr::UsdGeomSphere& sphere, const pxr::GfMatrix4d& world);
  virtual ~Sphere() {};

  void SetRadius(double radius){_radius = radius;};
  double GetRadius() {return _radius;};
  pxr::GfVec3f GetCenter(){return pxr::GfVec3f(GetMatrix().GetRow3(3));};

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  float SignedDistance(const pxr::GfVec3f& point) const override;

protected:
  Geometry::DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;

  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& time=pxr::UsdTimeCode::Default()) override;

private:
  double                    _radius;  

};

class Cube : public Geometry {
public:
  Cube(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Cube(const pxr::UsdGeomCube& sphere, const pxr::GfMatrix4d& world);
  virtual ~Cube() {};

  void SetSize(double size){_size = size;};
  double GetSize() {return _size;};

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  float SignedDistance(const pxr::GfVec3f& point) const override;

protected:
  Geometry::DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;

  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& time=pxr::UsdTimeCode::Default()) override;

private:
  double                    _size;  

};

class Cone : public Geometry {
public:
  Cone(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Cone(const pxr::UsdGeomCone& cone, const pxr::GfMatrix4d& world);
  virtual ~Cone() {};

  void SetRadius(double radius){_radius = radius;};
  double GetRadius() {return _radius;};
  void SetHeight(double height){_height = height;};
  double GetHeight() {return _height;};
  void SetAxis(const pxr::TfToken &axis){_axis = axis;};
  const pxr::TfToken& GetAxis() {return _axis;};

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

protected:
  Geometry::DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;
  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& time=pxr::UsdTimeCode::Default()) override;

private:
  double                   _radius;  
  double                   _height;
  pxr::TfToken             _axis;

};

class Cylinder : public Geometry {
public:
  Cylinder(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Cylinder(const pxr::UsdGeomCylinder& cylinder, const pxr::GfMatrix4d& world);
  virtual ~Cylinder() {};

  void SetRadius(double radius){_radius = radius;};
  double GetRadius() {return _radius;};
  void SetHeight(double height){_height = height;};
  double GetHeight() {return _height;};
  void SetAxis(const pxr::TfToken &axis){_axis = axis;};
  const pxr::TfToken& GetAxis() {return _axis;};

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

protected:
  Geometry::DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;
  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& time=pxr::UsdTimeCode::Default()) override;

private:
  double                    _radius;  
  double                    _height;
  pxr::TfToken             _axis;
};

class Capsule : public Geometry {
public:
  Capsule(const pxr::GfMatrix4d& xfo=pxr::GfMatrix4d(1.0));
  Capsule(const pxr::UsdGeomCapsule& sphere, const pxr::GfMatrix4d& world);
  virtual ~Capsule() {};

  void SetRadius(double radius){_radius = radius;};
  double GetRadius() {return _radius;};
  void SetHeight(double height){_height = height;};
  double GetHeight() {return _height;};
  void SetAxis(const pxr::TfToken &axis){_axis = axis;};
  const pxr::TfToken& GetAxis() {return _axis;};

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  float SignedDistance(const pxr::GfVec3f& point) const override;

protected:
  Geometry::DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) override;
  void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& time=pxr::UsdTimeCode::Default()) override;
  

private:
  double                   _radius;  
  double                   _height;
  pxr::TfToken             _axis;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_IMPLICIT_H
