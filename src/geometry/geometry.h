#ifndef AMN_GEOMETRY_GEOMETRY_H
#define AMN_GEOMETRY_GEOMETRY_H


#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <float.h>
#include "triangle.h"

AMN_NAMESPACE_OPEN_SCOPE

enum GeomInterpolation : short {
  GeomInterpolationConstant = 0,
  GeomInterpolationUniform,
  GeomInterpolationVarying,
  GeomInterpolationVertex,
  GeomInterpolationFaceVarying,

  GeomInterpolationCount
};

class Hit;
class Geometry {
public:
  enum Type {
    POINT,
    CURVE,
    MESH,
    STROKE,
    INSTANCER
  };

  Geometry();
  Geometry(const Geometry* other, bool normalize);
  virtual ~Geometry();

  short GetType() { return _type; };
  const pxr::VtArray<pxr::GfVec3f>& GetPositions() const {return _position;};
  const pxr::VtArray<pxr::GfVec3f>& GetNormals() const {return _normal;};

  pxr::VtArray<pxr::GfVec3f>& GetPositions() {return _position;};
  pxr::VtArray<pxr::GfVec3f>& GetNormals() {return _normal;};

  const pxr::GfVec3f* GetPositionsCPtr() const {return &_position[0];};
  const pxr::GfVec3f* GetNormalsCPtr() const {return &_normal[0];};

  pxr::GfVec3f GetPosition(uint32_t index) const;
  pxr::GfVec3f GetNormal(uint32_t index) const;

  uint32_t GetNumPoints()const {return _numPoints;};

  void Init(const pxr::VtArray<pxr::GfVec3f>& positions);
  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);
  void Normalize();
  void ComputeBoundingBox();
  pxr::GfBBox3d& GetBoundingBox() { return _bbox; };
  const pxr::GfBBox3d& GetBoundingBox() const { return _bbox; };

  bool IsInitialized(){return _initialized;};
  void SetInitialized(bool initialized){_initialized = initialized;};

  // query 3d position on geometry
  virtual bool Raycast(const pxr::GfRay& ray, Hit* hit,
    double maxDistance=-1.0, double* minDistance=NULL) const = 0;
  virtual bool Closest(const pxr::GfVec3f& point, Hit* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const = 0;

protected:
  // infos
  short                               _type;
  uint32_t                            _numPoints;

  // vertex data
  pxr::VtArray<pxr::GfVec3f>          _position;
  pxr::VtArray<pxr::GfVec3f>          _normal;

  // bounding box
  pxr::GfBBox3d                       _bbox;
  bool _initialized;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif
