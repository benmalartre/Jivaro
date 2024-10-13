#ifndef JVR_GEOMETRY_DEFORMABLE_H
#define JVR_GEOMETRY_DEFORMABLE_H

#include "../geometry/point.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

class Deformable : public Geometry {
public:
  Deformable(short type, const pxr::GfMatrix4d& matrix=pxr::GfMatrix4d(1.0));
  Deformable(const pxr::UsdPrim& prim, const pxr::GfMatrix4d& matrix=pxr::GfMatrix4d(1.0));
  virtual ~Deformable() {};

  size_t GetNumPoints()const override {return _positions.size();};

  const pxr::VtArray<pxr::GfVec3f>& GetPrevious() const {return _previous;};
  const pxr::VtArray<pxr::GfVec3f>& GetPositions() const {return _positions;};
  const pxr::VtArray<pxr::GfVec3f>& GetNormals() const {return _normals;};
  const pxr::VtArray<pxr::GfVec3f>& GetColors() const { return _colors; };
  const pxr::VtArray<float>& GetWidths() const { return _widths; };

  pxr::VtArray<pxr::GfVec3f>& GetPrevious() {return _previous;};
  pxr::VtArray<pxr::GfVec3f>& GetPositions() {return _positions;};
  pxr::VtArray<pxr::GfVec3f>& GetNormals() {return _normals;};
  pxr::VtArray<pxr::GfVec3f>& GetColors() { return _colors; };
  pxr::VtArray<float>& GetWidths() { return _widths; };

  pxr::GfVec3f* GetPreviousPtr() { return &_previous[0]; };
  pxr::GfVec3f* GetPositionsPtr() { return &_positions[0]; };
  pxr::GfVec3f* GetNormalsPtr() { return &_normals[0]; };
  pxr::GfVec3f* GetColorsPtr() { return &_colors[0]; };
  float* GetWidthsPtr() { return &_widths[0]; };

  const pxr::GfVec3f* GetPreviousCPtr() const {return &_previous[0];};
  const pxr::GfVec3f* GetPositionsCPtr() const {return &_positions[0];};
  const pxr::GfVec3f* GetNormalsCPtr() const {return &_normals[0];};
  const pxr::GfVec3f* GetColorsCPtr() const { return &_colors[0]; };
  const float* GetWidthsCPtr() const { return &_widths[0]; };

  pxr::GfVec3f GetPrevious(uint32_t index) const;
  pxr::GfVec3f GetPosition(uint32_t index) const;
  pxr::GfVec3f GetNormal(uint32_t index) const;
  pxr::GfVec3f GetColor(uint32_t index)const;
  float GetWidth(uint32_t index) const;

  void SetPrevious(uint32_t index, const pxr::GfVec3f& position);
  void SetPosition(uint32_t index, const pxr::GfVec3f& position);
  void SetNormal(uint32_t index, const pxr::GfVec3f& normal);
  void SetWidth(uint32_t index, float normal);

  void AddPoint(const pxr::GfVec3f& pos, float radius, 
                const pxr::GfVec3f* normal=NULL, const pxr::GfVec3f* color=NULL);
  void RemovePoint(size_t index);
  void RemovePoints(size_t start, size_t end);
  void RemoveAllPoints();

  Point* GetPoint(size_t index){return &_points[index];};

  virtual void SetPositions(const pxr::GfVec3f* positions, size_t n);
  virtual void SetWidths(const float* widths, size_t n);
  virtual void SetColors(const pxr::GfVec3f* colors, size_t n);
  virtual void SetNormals(const pxr::GfVec3f* normals, size_t n);

  virtual void SetPositions(const pxr::VtArray<pxr::GfVec3f>& positions);
  virtual void SetWidths(const pxr::VtArray<float>& widths);
  virtual void SetColors(const pxr::VtArray<pxr::GfVec3f>& colors);
  virtual void SetNormals(const pxr::VtArray<pxr::GfVec3f>& normals);

  virtual void Normalize();
  virtual void ComputeBoundingBox() override;

  Point Get(uint32_t index);

  bool HaveNormals() const { return _haveNormals; };
  bool HaveColors() const { return _haveColors; };
  bool HaveWidths() const { return _haveWidths; };

  // query 3d position on geometry
  virtual bool Raycast(const pxr::GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };
  virtual bool Closest(const pxr::GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };

protected:
  virtual void _ValidateNumPoints(size_t n);

  // todo have extensible list of attribute referenced by their name
  // with some predefined and other we can append on demand
  //std::map<pxr::TfToken, Attribute>  _attributes;
  
  // vertex data
  pxr::VtArray<pxr::GfVec3f>          _previous;
  pxr::VtArray<pxr::GfVec3f>          _positions;
  bool                                _haveNormals;
  pxr::VtArray<pxr::GfVec3f>          _normals;
  bool                                _haveColors;
  pxr::VtArray<pxr::GfVec3f>          _colors;
  bool                                _haveWidths;
  pxr::VtArray<float>                 _widths;
  pxr::VtArray<Point>                 _points;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
