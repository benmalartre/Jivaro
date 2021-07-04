// Stroke
//----------------------------------------------
#include "stroke.h"
#include "utils.h"

AMN_NAMESPACE_OPEN_SCOPE

Stroke::~Stroke()
{
};

Stroke::Stroke()
  : Geometry()
{
  _initialized = false;
  _numLines = 0;
  _type = STROKE;
}

Stroke::Stroke(const Stroke* other, bool normalize)
  : Geometry(other, normalize)
{
  _initialized = true;
  _numLines = other->_numLines;
  _type = STROKE;

  _raw = other->_raw;
  _lines.resize(_numLines);
  for(size_t i = 0; i < _numLines; ++i) {
    _lines[i].points = other->_lines[i].points;
    _lines[i].widths = other->_lines[i].widths;
  }
}

void Stroke::SetDisplayColor(GeomInterpolation interp, 
  const pxr::VtArray<pxr::GfVec3f>& colors) 
{
  _colorsInterpolation = interp;
  _colors = colors;
}

uint32_t Stroke::GetNumCVs(uint32_t lineIndex)const
{
  if(lineIndex >= _lines.size())
    return 0;
  return _lines[lineIndex].points.size();
}

float Stroke::GetSegmentLength(uint32_t lineIndex, uint32_t segmentIndex)
{
  /*
  size_t numCurves = _cvCounts.size();
  if(curveIndex >= numCurves)
    return -1.f;
  uint32_t numCurveSegments = _cvCounts[curveIndex];
  if(segmentIndex >= numCurveSegments)
    return -1.f;

  size_t baseCvIndex = segmentIndex;
  for(size_t i=0; i < curveIndex - 1; ++i)baseCvIndex += _cvCounts[i];

  return (_positions[segmentIndex] - _positions[segmentIndex + 1]).GetLength();
  */
 return 0.f;
}


AMN_NAMESPACE_CLOSE_SCOPE