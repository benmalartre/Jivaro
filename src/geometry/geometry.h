#ifndef JVR_GEOMETRY_GEOMETRY_H
#define JVR_GEOMETRY_GEOMETRY_H

#include <float.h>

#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>

#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE

enum GeomInterpolation : short {
  GeomInterpolationConstant = 0,
  GeomInterpolationUniform,
  GeomInterpolationVarying,
  GeomInterpolationVertex,
  GeomInterpolationFaceVarying,

  GeomInterpolationCount
};

class Location;

class Geometry {
public:
  enum Type {
    INVALID,
    XFORM,
    PLANE,
    SPHERE,
    CUBE,
    CONE, 
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

  Geometry();
  Geometry(short type, const pxr::GfMatrix4d& world);
  Geometry(const Geometry* other, short type);
  virtual ~Geometry() {};

  short GetType() const { return _type; };
  virtual size_t GetNumPoints() const {return 1;};

  void SetWirecolor(const pxr::GfVec3f& wirecolor){_wirecolor=wirecolor;};
  const pxr::GfVec3f& GetWirecolor() { return _wirecolor; };

  void SetMatrix(const pxr::GfMatrix4d& matrix);
  const pxr::GfMatrix4d& GetMatrix() const { return _matrix; };
  const pxr::GfMatrix4d& GetInverseMatrix() const { return _invMatrix; };

  virtual void ComputeBoundingBox() {};
  pxr::GfBBox3d& GetBoundingBox() { return _bbox; };
  const pxr::GfBBox3d& GetBoundingBox() const { return _bbox; };

  virtual DirtyState Sync(pxr::UsdPrim& prim, const pxr::GfMatrix4d& matrix, float time);

  // query 3d position on geometry
  virtual bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance=-1.0, double* minDistance=NULL) const {return false;};
  virtual bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const {return false;};

protected:
  virtual DirtyState _Sync(pxr::UsdPrim& prim, 
    const pxr::GfMatrix4d& matrix, float time) { return DirtyState::CLEAN;};

  template<typename T>
  DirtyState _GetAttrValue(pxr::UsdPrim& prim, const pxr::TfToken& name, 
    float time, T *value);

  // infos
  short                               _type;
  pxr::SdfPath                        _path;

  // bounding box
  pxr::GfMatrix4d                     _matrix;
  pxr::GfMatrix4d                     _invMatrix;
  pxr::GfBBox3d                       _bbox;
  pxr::GfVec3f                        _wirecolor;
};

template<typename T>
Geometry::DirtyState
Geometry::_GetAttrValue(pxr::UsdPrim& prim, const pxr::TfToken& name, float time, T *value)
{
  pxr::UsdAttribute attr = prim.GetAttribute(name);
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
