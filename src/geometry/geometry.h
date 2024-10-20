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
  Geometry(int type, const pxr::GfMatrix4d& world);
  Geometry(const pxr::UsdPrim& other, const pxr::GfMatrix4d& world);
  virtual ~Geometry() {};

  int GetType() const { return _type; };
  virtual size_t GetNumPoints() const {return 1;};
  pxr::UsdPrim& GetPrim(){return _prim;};
  const pxr::UsdPrim& GetPrim() const {return _prim;};

  template<typename T>
  DirtyState GetAttributeValue(const pxr::TfToken& name, 
    const pxr::UsdTimeCode& time, T* value);


  bool IsInput(){return BITMASK_CHECK(_mode, Mode::INPUT);};
  bool IsOutput(){return BITMASK_CHECK(_mode, Mode::OUTPUT);};
  void SetInputOnly() {_mode = Mode::INPUT;};
  void SetOutputOnly() {_mode = Mode::OUTPUT;};
  void SetInputOutput() {_mode = Mode::INPUT|Mode::OUTPUT;};
  
  void SetWirecolor(const pxr::GfVec3f& wirecolor){_wirecolor=wirecolor;};
  const pxr::GfVec3f& GetWirecolor() { return _wirecolor; };

  void SetMatrix(const pxr::GfMatrix4d& matrix);
  const pxr::GfMatrix4d& GetMatrix() const { return _matrix; };
  const pxr::GfMatrix4d& GetPreviousMatrix() const { return _prevMatrix; };
  const pxr::GfMatrix4d& GetInverseMatrix() const { return _invMatrix; };

  const pxr::GfVec3d GetTranslate();
  const pxr::GfVec3d GetScale();
  const pxr::GfQuatd GetRotation();

  const pxr::GfVec3f GetTorque() const;
  const pxr::GfVec3f GetVelocity() const;

  virtual void ComputeBoundingBox() {};
  void SetBoundingBox(const pxr::GfRange3d &range){ _bbox.Set(range, _matrix);};
  const pxr::GfBBox3d GetBoundingBox(bool worldSpace=true) const;

  void SetPrim(const pxr::UsdPrim& prim){_prim = prim;};
  virtual DirtyState Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default());
  virtual void Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default());

  // query 3d position on geometry
  virtual bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance=-1.0, double* minDistance=NULL) const {return false;};
  virtual bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const {return false;};

  virtual float SignedDistance(const pxr::GfVec3f& point) const {return 0.f;};

protected:
  void _ComputeVelocity();
  virtual DirtyState _Sync(const pxr::GfMatrix4d& matrix, 
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) { return DirtyState::CLEAN;};

  virtual void _Inject(const pxr::GfMatrix4d& parent,
    const pxr::UsdTimeCode& code=pxr::UsdTimeCode::Default()) = 0;

  // infos
  short                               _mode;
  int                                 _type;
  pxr::SdfPath                        _path;
  pxr::UsdPrim                        _prim;

  // bounding box
  pxr::GfMatrix4d                     _matrix;
  pxr::GfMatrix4d                     _prevMatrix;
  pxr::GfMatrix4d                     _invMatrix;
  pxr::GfVec3f                        _velocity;  // positional velocity
  pxr::GfVec3f                        _torque;    // rotational velocity
  pxr::GfBBox3d                       _bbox;
  pxr::GfVec3f                        _wirecolor;
};

template<typename T>
Geometry::DirtyState
Geometry::GetAttributeValue(const pxr::TfToken& name, 
  const pxr::UsdTimeCode& time, T *value)
{
  pxr::UsdAttribute attr = _prim.GetAttribute(name);
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
