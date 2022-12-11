#ifndef JVR_GEOMETRY_GEOMETRY_H
#define JVR_GEOMETRY_GEOMETRY_H


#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <float.h>

#include "../geometry/point.h"
#include "../geometry/edge.h"
#include "../geometry/triangle.h"

JVR_NAMESPACE_OPEN_SCOPE

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

  Geometry(short type);
  Geometry(const Geometry* other, short type, bool normalize);
  virtual ~Geometry() {};

  short GetType() { return _type; };
  const pxr::VtArray<pxr::GfVec3f>& GetPositions() const {return _positions;};
  const pxr::VtArray<pxr::GfVec3f>& GetNormals() const {return _normals;};
  const pxr::VtArray<float>& GetRadius() const { return _radius; };

  pxr::VtArray<pxr::GfVec3f>& GetPositions() {return _positions;};
  pxr::VtArray<pxr::GfVec3f>& GetNormals() {return _normals;};
  pxr::VtArray<float>& GetRadius() { return _radius; };

  const pxr::GfVec3f* GetPositionsCPtr() const {return &_positions[0];};
  const pxr::GfVec3f* GetNormalsCPtr() const {return &_normals[0];};
  const float* GetRadiusCPtr() const { return &_radius[0]; };

  pxr::GfVec3f GetPosition(uint32_t index) const;
  pxr::GfVec3f GetNormal(uint32_t index) const;
  float GetRadius(uint32_t index) const;

  void SetPosition(uint32_t index, const pxr::GfVec3f& position);
  void SetNormal(uint32_t index, const pxr::GfVec3f& normal);
  void SetRadius(uint32_t index, float normal);

  uint32_t GetNumPoints()const {return _numPoints;};

  void Init(const pxr::VtArray<pxr::GfVec3f>& positions);
  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);
  void SetPositions(pxr::GfVec3f* positions, size_t n);
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
  pxr::VtArray<Point>                 _points;

  // vertex data
  pxr::VtArray<pxr::GfVec3f>          _positions;
  pxr::VtArray<pxr::GfVec3f>          _normals;
  pxr::VtArray<float>                 _radius;


  // bounding box
  pxr::GfBBox3d                       _bbox;
  bool _initialized;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_GEOMETRY_H
