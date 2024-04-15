#ifndef JVR_GEOMETRY_DEFORMABLE_H
#define JVR_GEOMETRY_DEFORMABLE_H


#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/points.h>
#include <float.h>
#include "../geometry/point.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

class Deformable : public Geometry {
public:
  Deformable(short type, const pxr::GfMatrix4d& matrix=pxr::GfMatrix4d(1.f));
  Deformable(const Deformable* other, bool normalize = true);
  virtual ~Deformable() {};

  size_t GetNumPoints()const override {return _positions.size();};

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

  void AddPoint(const pxr::GfVec3f& pos);
  void RemovePoint(size_t index);
  void RemoveAllPoints();

  void Init(const pxr::VtArray<pxr::GfVec3f>& positions);
  void Init(const pxr::VtArray<pxr::GfVec3f>& positions, 
    const pxr::VtArray<float>& radius);

  void Update(const pxr::VtArray<pxr::GfVec3f>& positions);
  void Update(const pxr::VtArray<float>& radius);
  void Update(const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<float>& radius);

  void SetPositions(const pxr::GfVec3f* positions, size_t n);
  void SetRadii(const float* radii, size_t n);
  void SetColors(const pxr::GfVec3f* colors, size_t n);

  void SetPositions(const pxr::VtArray<pxr::GfVec3f>& positions);
  void SetRadii(const pxr::VtArray<float>& radii);
  void SetColors(const pxr::VtArray<pxr::GfVec3f>& colors);

  void Normalize();
  void ComputeBoundingBox() override;

  Point Get(uint32_t index);

  bool HaveNormals() const { return _haveNormals; };
  bool HaveColors() const { return _haveColors; };
  bool HaveRadius() const { return _haveRadius; };

  // query 3d position on geometry
  bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };
  bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };

protected:
  // vertex data
  pxr::VtArray<pxr::GfVec3f>          _positions;
  bool                                _haveNormals;
  pxr::VtArray<pxr::GfVec3f>          _normals;
  bool                                _haveColors;
  pxr::VtArray<pxr::GfVec3f>          _colors;
  bool                                _haveRadius;
  pxr::VtArray<float>                 _radius;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
