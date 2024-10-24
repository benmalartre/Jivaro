#ifndef JVR_GEOMETRY_GEOMETRY_H
#define JVR_GEOMETRY_GEOMETRY_H

#include <limits>
#include <float.h>

#include "../common.h"

#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/quath.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/ray.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>

JVR_NAMESPACE_OPEN_SCOPE

class Location;

class Geometry {
public:
  static float FrameDuration; 
  static void SetFrameDuration(float duration) { FrameDuration = duration; };

  enum Mode : short {
    INPUT = 1,
    OUTPUT = 2
  };

  enum Type : int {
    INVALID,
    XFORM,
    SOLVER,
    PLANE,
    SPHERE,
    CUBE,
    CONE, 
    CYLINDER,
    CAPSULE,
    POINT,
    CURVE,
    MESH,
    STROKE,
    INSTANCER,
    VOXEL
  };

  enum DirtyState : size_t {
    CLEAN     = 0,
    TRANSFORM = 1,
    DEFORM    = 2,
    TOPOLOGY  = 4,
    ATTRIBUTE = 8,
    ALLDIRTY  = 15
  };

  enum Interpolation : short {
    CONSTANT = 0,
    UNIFORM,
    VARYING,
    VERTEX,
    FACEVARYING,
    COUNT
  };

  Geometry();
  Geometry(int type, const GfMatrix4d& world);
  Geometry(const UsdPrim& other, const GfMatrix4d& world);
  virtual ~Geometry() {};

  int GetType() const { return _type; };
  virtual size_t GetNumPoints() const {return 1;};
  UsdPrim& GetPrim(){return _prim;};
  const UsdPrim& GetPrim() const {return _prim;};

  template<typename T>
  DirtyState GetAttributeValue(const TfToken& name, 
    const UsdTimeCode& time, T* value);


  bool IsInput(){return BITMASK_CHECK(_mode, Mode::INPUT);};
  bool IsOutput(){return BITMASK_CHECK(_mode, Mode::OUTPUT);};
  void SetInputOnly() {_mode = Mode::INPUT;};
  void SetOutputOnly() {_mode = Mode::OUTPUT;};
  void SetInputOutput() {_mode = Mode::INPUT|Mode::OUTPUT;};
  
  void SetWirecolor(const GfVec3f& wirecolor){_wirecolor=wirecolor;};
  const GfVec3f& GetWirecolor() { return _wirecolor; };

  void SetMatrix(const GfMatrix4d& matrix);
  const GfMatrix4d& GetMatrix() const { return _matrix; };
  const GfMatrix4d& GetPreviousMatrix() const { return _prevMatrix; };
  const GfMatrix4d& GetInverseMatrix() const { return _invMatrix; };

  const GfVec3d GetTranslate();
  const GfVec3d GetScale();
  const GfQuatd GetRotation();

  const GfVec3f GetTorque() const;
  const GfVec3f GetVelocity() const;

  virtual void ComputeBoundingBox() {};
  void SetBoundingBox(const GfRange3d &range){ _bbox.Set(range, _matrix);};
  const GfBBox3d GetBoundingBox(bool worldSpace=true) const;

  void SetPrim(const UsdPrim& prim){_prim = prim;};
  virtual DirtyState Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default());
  virtual void Inject(const GfMatrix4d& parent,
    const UsdTimeCode& code=UsdTimeCode::Default());

  // query 3d position on geometry
  virtual bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance=-1.0, double* minDistance=NULL) const {return false;};
  virtual bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const {return false;};

  virtual float SignedDistance(const GfVec3f& point) const {return 0.f;};

protected:
  void _ComputeVelocity();
  virtual DirtyState _Sync(const GfMatrix4d& matrix, 
    const UsdTimeCode& code=UsdTimeCode::Default()) { return DirtyState::CLEAN;};

  virtual void _Inject(const GfMatrix4d& parent,
    const UsdTimeCode& code=UsdTimeCode::Default()) = 0;

  // infos
  short                               _mode;
  int                                 _type;
  SdfPath                        _path;
  UsdPrim                        _prim;

  // bounding box
  GfMatrix4d                     _matrix;
  GfMatrix4d                     _prevMatrix;
  GfMatrix4d                     _invMatrix;
  GfVec3f                        _velocity;  // positional velocity
  GfVec3f                        _torque;    // rotational velocity
  GfBBox3d                       _bbox;
  GfVec3f                        _wirecolor;
};

template<typename T>
Geometry::DirtyState
Geometry::GetAttributeValue(const TfToken& name, 
  const UsdTimeCode& time, T *value)
{
  UsdAttribute attr = _prim.GetAttribute(name);
  if(!attr.IsValid())return DirtyState::CLEAN;

  T tmp;
  attr.Get(&tmp, time);
  if(tmp != *value) {
    *value = tmp;
    return DirtyState::ATTRIBUTE;
  }
  return DirtyState::CLEAN;
}

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_GEOMETRY_H
