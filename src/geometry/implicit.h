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
  Xform(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Xform(const UsdGeomXform& xform, const GfMatrix4d& world);
  virtual ~Xform() {};

  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& code=UsdTimeCode::Default()) override{};

};

class Plane : public Geometry {
public:
  Plane(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Plane(const UsdGeomPlane& plane, const GfMatrix4d& world);
  virtual ~Plane() {};


  GfVec3f GetNormal();
  GfVec3f GetOrigin();

  GfPlane Get();

  // query 3d position on geometry
  bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  float SignedDistance(const GfVec3f& point) const override;

protected:
  Geometry::DirtyState _Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default()) override;

  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& time=UsdTimeCode::Default()) override;

private:
  TfToken                _axis;
  GfVec3f                _normal;
  double                      _width;
  double                      _length;
  bool                        _doubleSided;
};

class Sphere : public Geometry {
public:
  Sphere(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Sphere(const UsdGeomSphere& sphere, const GfMatrix4d& world);
  virtual ~Sphere() {};

  void SetRadius(double radius){_radius = radius;};
  double GetRadius() {return _radius;};
  GfVec3f GetCenter(){return GfVec3f(GetMatrix().GetRow3(3));};

  // query 3d position on geometry
  bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  float SignedDistance(const GfVec3f& point) const override;

protected:
  Geometry::DirtyState _Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default()) override;

  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& time=UsdTimeCode::Default()) override;

private:
  double                    _radius;  

};

class Cube : public Geometry {
public:
  Cube(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Cube(const UsdGeomCube& sphere, const GfMatrix4d& world);
  virtual ~Cube() {};

  void SetSize(double size){_size = size;};
  double GetSize() {return _size;};

  // query 3d position on geometry
  bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  float SignedDistance(const GfVec3f& point) const override;

protected:
  Geometry::DirtyState _Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default()) override;

  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& time=UsdTimeCode::Default()) override;

private:
  double                    _size;  

};

class Cone : public Geometry {
public:
  Cone(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Cone(const UsdGeomCone& cone, const GfMatrix4d& world);
  virtual ~Cone() {};

  void SetRadius(double radius){_radius = radius;};
  double GetRadius() {return _radius;};
  void SetHeight(double height){_height = height;};
  double GetHeight() {return _height;};
  void SetAxis(const TfToken &axis){_axis = axis;};
  const TfToken& GetAxis() {return _axis;};

  // query 3d position on geometry
  bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

protected:
  Geometry::DirtyState _Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default()) override;
  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& time=UsdTimeCode::Default()) override;

private:
  double                   _radius;  
  double                   _height;
  TfToken             _axis;

};

class Cylinder : public Geometry {
public:
  Cylinder(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Cylinder(const UsdGeomCylinder& cylinder, const GfMatrix4d& world);
  virtual ~Cylinder() {};

  void SetRadius(double radius){_radius = radius;};
  double GetRadius() {return _radius;};
  void SetHeight(double height){_height = height;};
  double GetHeight() {return _height;};
  void SetAxis(const TfToken &axis){_axis = axis;};
  const TfToken& GetAxis() {return _axis;};

  // query 3d position on geometry
  bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;

protected:
  Geometry::DirtyState _Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default()) override;
  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& time=UsdTimeCode::Default()) override;

private:
  double                    _radius;  
  double                    _height;
  TfToken             _axis;
};

class Capsule : public Geometry {
public:
  Capsule(const GfMatrix4d& xfo=GfMatrix4d(1.0));
  Capsule(const UsdGeomCapsule& sphere, const GfMatrix4d& world);
  virtual ~Capsule() {};

  void SetRadius(double radius){_radius = radius;};
  double GetRadius() {return _radius;};
  void SetHeight(double height){_height = height;};
  double GetHeight() {return _height;};
  void SetAxis(const TfToken &axis){_axis = axis;};
  const TfToken& GetAxis() {return _axis;};

  // query 3d position on geometry
  bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override;
  float SignedDistance(const GfVec3f& point) const override;

protected:
  Geometry::DirtyState _Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default()) override;
  void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& time=UsdTimeCode::Default()) override;
  

private:
  double                   _radius;  
  double                   _height;
  TfToken             _axis;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_IMPLICIT_H
