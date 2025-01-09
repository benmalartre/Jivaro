#ifndef JVR_GEOMETRY_DEFORMABLE_H
#define JVR_GEOMETRY_DEFORMABLE_H

#include "../geometry/point.h"
#include "../geometry/geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

class Deformable : public Geometry {
public:
  Deformable(short type, const GfMatrix4d& matrix=GfMatrix4d(1.0));
  Deformable(const UsdPrim& prim, const GfMatrix4d& matrix=GfMatrix4d(1.0));
  virtual ~Deformable() {};

  size_t GetNumPoints()const override {return _positions.size();};

  const VtArray<GfVec3f>& GetPrevious() const {return _previous;};
  const VtArray<GfVec3f>& GetPositions() const {return _positions;};
  const VtArray<GfVec3f>& GetNormals() const {return _normals;};
  const VtArray<GfVec3f>& GetColors() const { return _colors; };
  const VtArray<float>& GetWidths() const { return _widths; };

  VtArray<GfVec3f>& GetPrevious() {return _previous;};
  VtArray<GfVec3f>& GetPositions() {return _positions;};
  VtArray<GfVec3f>& GetNormals() {return _normals;};
  VtArray<GfVec3f>& GetColors() { return _colors; };
  VtArray<float>& GetWidths() { return _widths; };

  GfVec3f* GetPreviousPtr() { return &_previous[0]; };
  GfVec3f* GetPositionsPtr() { return &_positions[0]; };
  GfVec3f* GetNormalsPtr() { return &_normals[0]; };
  GfVec3f* GetColorsPtr() { return &_colors[0]; };
  float* GetWidthsPtr() { return &_widths[0]; };

  const GfVec3f* GetPreviousCPtr() const {return &_previous[0];};
  const GfVec3f* GetPositionsCPtr() const {return &_positions[0];};
  const GfVec3f* GetNormalsCPtr() const {return &_normals[0];};
  const GfVec3f* GetColorsCPtr() const { return &_colors[0]; };
  const float* GetWidthsCPtr() const { return &_widths[0]; };

  GfVec3f GetPrevious(uint32_t index) const;
  GfVec3f GetPosition(uint32_t index) const;
  GfVec3f GetNormal(uint32_t index) const;
  GfVec3f GetColor(uint32_t index)const;
  GfVec3f GetVelocity(uint32_t index)const;
  float GetWidth(uint32_t index) const;

  void SetPrevious(uint32_t index, const GfVec3f& position);
  void SetPosition(uint32_t index, const GfVec3f& position);
  void SetNormal(uint32_t index, const GfVec3f& normal);
  void SetWidth(uint32_t index, float normal);

  void AddPoint(const GfVec3f& pos, float radius, 
                const GfVec3f* normal=NULL, const GfVec3f* color=NULL);
  void RemovePoint(size_t index);
  void RemovePoints(size_t start, size_t end);
  void RemoveAllPoints();

  Point* GetPoint(size_t index){return &_points[index];};

  virtual void SetPositions(const GfVec3f* positions, size_t n);
  virtual void SetWidths(const float* widths, size_t n);
  virtual void SetColors(const GfVec3f* colors, size_t n);
  virtual void SetNormals(const GfVec3f* normals, size_t n);

  virtual void SetPositions(const VtArray<GfVec3f>& positions);
  virtual void SetWidths(const VtArray<float>& widths);
  virtual void SetColors(const VtArray<GfVec3f>& colors);
  virtual void SetNormals(const VtArray<GfVec3f>& normals);

  virtual void Normalize();
  virtual void ComputeBoundingBox() override;

  Point Get(uint32_t index);

  bool HaveNormals() const { return _haveNormals; };
  bool HaveColors() const { return _haveColors; };
  bool HaveWidths() const { return _haveWidths; };

  // query 3d position on geometry
  virtual bool Raycast(const GfRay& ray, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };
  virtual bool Closest(const GfVec3f& point, Location* hit,
    double maxDistance = -1.0, double* minDistance = NULL) const override {
    return false;
  };

protected:
  virtual void _ValidateNumPoints(size_t n);

  // todo have extensible list of attribute referenced by their name
  // with some predefined and other we can append on demand
  //std::map<TfToken, Attribute>  _attributes;
  
  // vertex data
  VtArray<GfVec3f>          _previous;
  VtArray<GfVec3f>          _positions;
  bool                      _haveNormals;
  VtArray<GfVec3f>          _normals;
  bool                      _haveColors;
  VtArray<GfVec3f>          _colors;
  bool                      _haveWidths;
  VtArray<float>            _widths;
  VtArray<Point>            _points;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
