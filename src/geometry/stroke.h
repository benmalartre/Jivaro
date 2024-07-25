#ifndef JVR_GEOMETRY_STROKE_H
#define JVR_GEOMETRY_STROKE_H


#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <float.h>
#include "geometry.h"

JVR_NAMESPACE_OPEN_SCOPE

struct StrokeLine {
  std::vector<pxr::GfVec3f> points;
  std::vector<float> widths;

  StrokeLine() {};
  StrokeLine(const std::vector<pxr::GfVec3f>& p, const std::vector<float>& w)
    : points(p), widths(w){};
};

struct StrokeInput {
  std::vector<pxr::GfVec3f> raw_points;
  std::vector<float> raw_widths;
  std::vector<pxr::GfVec3f> points;
  std::vector<float> widths;
  uint32_t baseIndex;

  void Clear();
  void Update(float threshold);
};

class Stroke : public Geometry {
public:
  enum Interaction {
    NONE,
    CREATE,
    INSERT,
    EDIT,
    REMOVE,
    DEL
  };
  Stroke();
  Stroke(const Stroke& other, bool normalize = true);
  virtual ~Stroke();

  const pxr::VtArray<StrokeLine>& GetLines() const { return _lines;};
  pxr::VtArray<StrokeLine>& GetLines() { return _lines;};

  void SetDisplayColor(Geometry::Interpolation interp, 
    const pxr::VtArray<pxr::GfVec3f>& colors);
  const pxr::VtArray<pxr::GfVec3f>& GetDisplayColor() const {return _colors;};
  Geometry::Interpolation GetDisplayColorInterpolation() const {
    return _colorsInterpolation;
  };

  uint32_t GetNumCVs(uint32_t lineIndex)const;

  float GetSegmentLength(uint32_t lineIndex, uint32_t segmentIndex);

  void StartLine();
  void AddPoint(const pxr::GfVec3f& position);
  void EndLine(bool closed);
  void Refine();


private:
  // infos
  uint32_t                    _numLines;

  // strokes
  pxr::VtArray<StrokeLine>    _lines;
  StrokeInput                 _input;
  short                       _interacting;

  // colors
  pxr::VtArray<pxr::GfVec3f>  _colors;
  Geometry::Interpolation     _colorsInterpolation;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif
