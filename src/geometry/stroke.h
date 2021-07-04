#ifndef AMN_GEOMETRY_STOKE_H
#define AMN_GEOMETRY_STOKE_H


#include "../common.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <float.h>
#include "geometry.h"

AMN_NAMESPACE_OPEN_SCOPE

struct StrokeLine {
  std::vector<pxr::GfVec3f> points;
  std::vector<pxr::GfVec3f> widths;
};

struct StrokeRaw {
  std::vector<pxr::GfVec3f> raw_points;
  std::vector<pxr::GfVec3f> raw_widths;
  std::vector<pxr::GfVec3f> points;
  std::vector<pxr::GfVec3f> widths;
  uint32_t breakIndex;
};

class Stroke : public Geometry {
public:
  Stroke();
  Stroke(const Stroke* other, bool normalize = true);
  ~Stroke();

  const pxr::VtArray<StrokeLine>& GetLines() const { return _lines;};
  pxr::VtArray<StrokeLine>& GetLines() { return _lines;};

  void SetDisplayColor(GeomInterpolation interp, 
    const pxr::VtArray<pxr::GfVec3f>& colors);
  const pxr::VtArray<pxr::GfVec3f>& GetDisplayColor() const {return _colors;};
  GeomInterpolation GetDisplayColorInterpolation() const {
    return _colorsInterpolation;
  };

  uint32_t GetNumCVs(uint32_t lineIndex)const;

  float GetSegmentLength(uint32_t lineIndex, uint32_t segmentIndex);

  void AddPoint(const pxr::GfVec3d& position);
  void Refine();


private:
  // infos
  uint32_t                    _numLines;

  // strokes
  pxr::VtArray<StrokeLine>    _lines;  
  StrokeRaw                   _raw;

  // colors
  pxr::VtArray<pxr::GfVec3f>  _colors;
  GeomInterpolation           _colorsInterpolation;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif
