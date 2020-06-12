#include "icon.h"
#include <limits>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../imgui/imgui_internal.h"

AMN_NAMESPACE_OPEN_SCOPE

float IconButtonUI::EPSILON = 0.0001;
float IconButtonUI::PI = 3.14159265358979;


void IconButtonUI::_AddVertex(const ImVec2& pos, const ImU32 color)
{
  _vertices.push_back({
      pos,
      pxr::GfVec2f(0.f,0.f),
      color
    });
}

void IconButtonUI::_CreateRoundCap(const ImVec2& center, const ImVec2& p0,
  const ImVec2& p1, const ImVec2& next, const ImU32 color)
{
  size_t vertexIdx = _vertices.size();
  float radius = 
    std::sqrtf(std::powf(center[0] - p0[0], 2) + std::powf(center[1] - p0[1], 2));

  float angle0 = std::atan2f((p1[1] - center[1]), (p1[0] - center[0]));
  float angle1 = std::atan2f((p0[1] - center[1]), (p0[0] - center[0]));

  float orgAngle0 = angle0;

  if (angle1 > angle0) {
    if ((angle1 - angle0) >= (PI - EPSILON)) {
      angle1 = angle1 - 2 * PI;
    }
  }
  else {
    if ((angle0 - angle1 >= PI - EPSILON)) {
      angle0 = angle0 - 2 * PI;
    }
  }

  float angleDiff = angle1 - angle0;

  if ((pxr::GfAbs(angleDiff)) >= (PI - EPSILON) && (pxr::GfAbs(angleDiff)) <= (PI + EPSILON)) {
    pxr::GfVec2f r1 = ImVec2(center[0] - next[0], center[1] - next[1]);
    if (r1[0] == 0) {
      if (r1[1]>0) {
        angleDiff = -angleDiff;
      }
    }
    else if (r1[0] >= -EPSILON) {
      angleDiff = -angleDiff;
    }
  }

  size_t nSegments = size_t(pxr::GfAbs(angleDiff * radius) / 7);
  nSegments++;

  float angleInc = angleDiff / nSegments;

  _AddVertex(center, color);
  size_t centerIdx = vertexIdx++;

  for (size_t s = 0; s < nSegments + 1; ++s) {
    _AddVertex(
      pxr::GfVec2f(
        center[0] + radius * pxr::GfCos(orgAngle0 + angleInc * s),
        center[1] + radius * pxr::GfSin(orgAngle0 + angleInc * s)), 
      color);

    if (s > 0) {
      _indices.push_back(centerIdx);
      _indices.push_back(centerIdx + s);
      _indices.push_back(centerIdx + s+1);
    }
  }
}

void IconButtonUI::_CreateTriangles(const ImVec2& p0, const ImVec2& p1,
  float width0, float width1, ImU32 c0, ImU32 c1) 
{
  size_t vertexIdx = _vertices.size();
  ImVec2 t(p1.x - p0.x, p1.y - p0.y);

  ImVec2 t0(-t.y, t.x);
  ImVec2 t1(-t.y, t.x);

  float l = std::sqrtf(ImLengthSqr(t));

  if (l != 0.f) {
    t0 = ImVec2(t0[0] / l * width0, t0[1] / l * width0);
    t1 = ImVec2(t1[0] / l * width1, t1[1] / l * width1);
  }

  _AddVertex(p0 + t0, c0);
  _AddVertex(p0 - t0, c0);
  _AddVertex(p1 + t1, c1);
  _AddVertex(p1 - t1, c1);

  _indices.push_back(vertexIdx);
  _indices.push_back(vertexIdx + 1);
  _indices.push_back(vertexIdx + 2);
  _indices.push_back(vertexIdx + 1);
  _indices.push_back(vertexIdx + 2);
  _indices.push_back(vertexIdx + 3);
  
}

void IconButtonUI::Build(IconItem* item, size_t n)
{
  _items = item;
  _numItems = n;
  for (size_t i = 0; i < n; ++i) {
    const size_t base = _vertices.size();
    const size_t cn = item[i].n;
    const ImVec2* cp = (ImVec2*)item[i].points;

    const float* cw = item[i].widths;
    const ImU32* cc = item[i].colors;
    if (cn < 2) continue;

 
    size_t last = item[i].flag & IconItemClosed ? cn : cn - 1;
    for (size_t j = 0; j < last; ++j) {
      size_t k = (j + 1) % cn;
       _CreateTriangles(cp[j], cp[k], cw[j], cw[k], cc[j], cc[k]);
    }

    ImVec2 p0, p1, p2;
    if (item[i].flag & IconItemClosed) {
      for (size_t j = 0; j < last; ++j) {
        p0 = _vertices[base + j * 4].pos;
        p1 = _vertices[base + j * 4 + 1].pos;
        p2 = cp[(j + 1)%cn];
        _CreateRoundCap(cp[j], p0, p1, p2, cc[j]);
      }
    }
    else {
      p0 = _vertices[base].pos;
      p1 = _vertices[base + 1].pos;
      p2 = cp[1];
      _CreateRoundCap(cp[0], p0, p1, p2, cc[1]);
      
      for (size_t j = 1; j < last - 1; ++j) {
        p0 = _vertices[base + j * 4].pos;
        p1 = _vertices[base + j*4+1].pos;
        p2 = cp[j +1];
        _CreateRoundCap(cp[j], p0, p1, p2, cc[j+1]);
      }

      p0 = _vertices[_vertices.size() - 1].pos;
      p1 = _vertices[_vertices.size() - 2].pos;
      p2 = cp[cn - 2];
      _CreateRoundCap(cp[cn - 1], p0, p1, p2, cc[cn-2]);
    }
  }
  _initialized = true;
}

void IconButtonUI::Draw(ImDrawList* drawList)
{
  const size_t indexCnt = _indices.size();
  const size_t vertexCnt = _vertices.size();

  drawList->PrimReserve(indexCnt, vertexCnt);
  for (size_t ii = 0; ii < indexCnt; ++ii) {
    drawList->_IdxWritePtr[ii] = (ImDrawIdx)_indices[ii] + drawList->_VtxCurrentIdx;
  }

  for (size_t iv = 0; iv < vertexCnt; ++iv) {
    drawList->_VtxWritePtr[iv] = _vertices[iv];
    drawList->_VtxWritePtr[iv].uv = drawList->_Data->TexUvWhitePixel;
  }

  drawList->_VtxCurrentIdx += (ImDrawIdx)vertexCnt;

  for (size_t n = 0; n < _numItems;++n) {
    
    const IconItem& item = _items[n];
    drawList->AddConvexPolyFilled((ImVec2*)&item.points[0], item.n, 0xFF0000FF);
  }
}

AMN_NAMESPACE_CLOSE_SCOPE