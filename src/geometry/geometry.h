#ifndef JVR_GEOMETRY_GEOMETRY_H
#define JVR_GEOMETRY_GEOMETRY_H

#include <float.h>

#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>

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

class Hit;
class Geometry {
public:
  enum Type {
    INVALID,
    POINT,
    CURVE,
    MESH,
    STROKE,
    INSTANCER,
    VOXEL
  };

  Geometry();
  Geometry(short type, const pxr::GfMatrix4d& world);
  Geometry(const Geometry* other, short type, bool normalize);
  virtual ~Geometry() {};

  short GetType() { return _type; };

  bool HasNormals() const { return _normals.size() > 0; };
  bool HasColors() const { return _colors.size() > 0; };
  bool HasRadius() const { return _radius.size() > 0; };

  const pxr::GfVec3f& GetWirecolor() { return _wirecolor; };
  const pxr::VtArray<pxr::GfVec3f>& GetPositions() const {return _positions;};
  const pxr::VtArray<pxr::GfVec3f>& GetNormals() const {return _normals;};
  const pxr::VtArray<pxr::GfVec3f>& GetColors() const { return _colors; };
  const pxr::VtArray<float>& GetRadius() const { return _radius; };

  pxr::VtArray<pxr::GfVec3f>& GetPositions() {return _positions;};
  pxr::VtArray<pxr::GfVec3f>& GetNormals() {return _normals;};
  pxr::VtArray<pxr::GfVec3f>& GetColors() { return _colors; };
  pxr::VtArray<float>& GetRadius() { return _radius; };

  pxr::GfVec3f* GetPositionsPtr() { return &_positions[0]; };
  pxr::GfVec3f* GetNormalsPtr() { return &_normals[0]; };
  pxr::GfVec3f* GetColorsPtr() { return &_colors[0]; };
  float* GetRadiusPtr() { return &_radius[0]; };

  const pxr::GfVec3f* GetPositionsCPtr() const {return &_positions[0];};
  const pxr::GfVec3f* GetNormalsCPtr() const {return &_normals[0];};
  const pxr::GfVec3f* GetColorsCPtr() const { return &_colors[0]; };
  const float* GetRadiusCPtr() const { return &_radius[0]; };

  pxr::GfVec3f GetPosition(uint32_t index) const;
  pxr::GfVec3f GetNormal(uint32_t index) const;
  pxr::GfVec3f GetColor(uint32_t index)const;
  float GetRadius(uint32_t index) const;

  void SetPosition(uint32_t index, const pxr::GfVec3f& position);
  void SetNormal(uint32_t index, const pxr::GfVec3f& normal);
  void SetRadius(uint32_t index, float normal);
  void SetMatrix(const pxr::GfMatrix4d& matrix) { _matrix = matrix; };

  size_t GetNumPoints()const {return _positions.size();};
  const pxr::GfMatrix4d& GetMatrix() const { return _matrix; };

  void AddPoint(const pxr::GfVec3f& pos);
  void RemovePoint(size_t index);

  void Init(const pxr::VtArray<pxr::GfVec3f>& positions);
  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);

  void SetPositions(const pxr::GfVec3f* positions, size_t n);
  void SetRadii(const float* radii, size_t n);
  void SetColors(const pxr::GfVec3f* colors, size_t n);
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

  // vertex data
  pxr::VtArray<pxr::GfVec3f>          _positions;
  pxr::VtArray<pxr::GfVec3f>          _normals;
  pxr::VtArray<pxr::GfVec3f>          _colors;
  pxr::VtArray<float>                 _radius;

  // bounding box
  pxr::GfMatrix4d                     _matrix;
  pxr::GfBBox3d                       _bbox;
  bool                                _initialized;
  pxr::GfVec3f                        _wirecolor;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_GEOMETRY_H
