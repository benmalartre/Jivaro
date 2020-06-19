#ifndef AMN_WIDGET_ICON_H
#define AMN_WIDGET_ICON_H
#pragma once

#include "../common.h"
#include <vector>
#include "../imgui/imgui.h"


AMN_NAMESPACE_OPEN_SCOPE

#define IconItemStroked 1
#define IconItemFilled  2
#define IconItemClosed  4

#define IconWidth  100;
#define IconHeight 100;

struct IconItem {
  char flag;
  size_t n;
  float* points;
  float* widths;
  ImU32* colors;
};

#define PlayIconN 1
static float PlayIconPoints[] = {
  20.f,20.f,20.f,80.f,80.f,50.f
};
static float PlayIconWidths[] = {
  6.f,5.f,4.f
};
static ImU32 PlayIconColors[] = {
  0xFF00FF00,
  0xFFFF0000,
  0xFF0000FF,
};

static IconItem PlayIcon[PlayIconN] = {
  {
    IconItemStroked | IconItemFilled | IconItemClosed,
    3,
    &PlayIconPoints[0],
    &PlayIconWidths[0],
    &PlayIconColors[0]
  } 
};

class IconButtonUI
{
public:
  IconButtonUI() : _initialized(false) {};
  ~IconButtonUI() {};

  void Build(IconItem* item, size_t n);
  void Draw(ImDrawList* drawList);
private:
  static float EPSILON;
  static float PI;
  void _AddVertex(const ImVec2& pos, const ImU32 color);
  void _CreateRoundCap(const ImVec2& center, const ImVec2& p0,
    const ImVec2& p1, const ImVec2& next, const ImU32 color);
  void _CreateTriangles(const ImVec2& p0, const ImVec2& p1,
    float width0, float width1, ImU32 c0, ImU32 c1);

  IconItem*               _items;
  size_t                  _numItems;
  std::vector<ImDrawVert> _vertices;
  std::vector<int>        _indices;
  bool                    _initialized;

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_WIDGET_ICON_H