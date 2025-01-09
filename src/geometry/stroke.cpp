// Stroke
//----------------------------------------------
#include "../geometry/stroke.h"
#include "../geometry/utils.h"

JVR_NAMESPACE_OPEN_SCOPE
void StrokeInput::Clear(){
  raw_points.clear();
  raw_widths.clear();
  points.clear();
  widths.clear();
}

void StrokeInput::Update(float threshold)
{
  size_t numRawPoints = raw_points.size();
  size_t numActivePoints = numRawPoints - baseIndex;
  std::vector<GfVec3f> deltas;
  std::vector<float> distances(numActivePoints - 1);
  std::vector<bool> preserve(numActivePoints);
  
  for(size_t i=1; i < numActivePoints - 1; ++i) {
    deltas[i] = raw_points[baseIndex + i + 1] - raw_points[baseIndex + i];
    distances[i] = deltas[i].GetLength();
  }
}


Stroke::~Stroke()
{
};

Stroke::Stroke()
  : Geometry(Geometry::STROKE, GfMatrix4d(1.0))
{
  _numLines = 0;
  _type = STROKE;
}

Stroke::Stroke(const Stroke& other, bool normalize)
  : Geometry(Geometry::STROKE, other.GetMatrix())
{
  _numLines = other._numLines;
  _type = STROKE;

  _input = other._input;
  _lines.resize(_numLines);
  for(size_t i = 0; i < _numLines; ++i) {
    _lines[i].points = other._lines[i].points;
    _lines[i].widths = other._lines[i].widths;
  }
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

void Stroke::StartLine()
{
  if(_input.points.size()) {
    _lines.push_back(StrokeLine(_input.points, _input.widths));
    _input.Clear();
  }
  _interacting = CREATE;
}

void Stroke::EndLine(bool closed)
{

}

void Stroke::AddPoint(const GfVec3f& position)
{
  if (_interacting == CREATE) {
    _input.raw_points.push_back(position);
    _input.Update(0.05f);
  }

}
  
void Stroke::Refine()
{

}


JVR_NAMESPACE_CLOSE_SCOPE