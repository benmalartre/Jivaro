#pragma once

#include "../common.h"
#include "../utils/utils.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>

AMN_NAMESPACE_OPEN_SCOPE
class View;

enum BORDER
{
  TOP     = 1,
  RIGHT   = 2,
  BOTTOM  = 4,
  LEFT    = 8
};

#define SPLITTER_THICKNESS 1.0

class Splitter
{
public: 
  Splitter():_pixels(NULL),_valid(false){
    _flags = 
      ImGuiWindowFlags_None
      | ImGuiWindowFlags_NoBackground
      | ImGuiWindowFlags_NoDecoration
      | ImGuiWindowFlags_NoInputs
      | ImGuiWindowFlags_NoMouseInputs
      | ImGuiWindowFlags_NoMove
      | ImGuiWindowFlags_NoResize
      | ImGuiWindowFlags_NoCollapse
      | ImGuiWindowFlags_NoNav
      | ImGuiWindowFlags_NoNavInputs
      | ImGuiWindowFlags_NoTitleBar
      | ImGuiWindowFlags_NoScrollbar;
  };
  
  ~Splitter(){
    if(_pixels)delete [] _pixels;
  };

  int* GetPixels(){return _pixels;};
  inline unsigned GetWidth(){return _width;};
  inline unsigned GetHeight(){return _height;};
  void RecurseBuildMap(View* view);
  void BuildMap(int width, int height);
  View* GetViewByIndex(int index);
  int Pick(int x, int y);
  void SetHorizontalCursor(){_cursor = ImGuiMouseCursor_ResizeEW;};
  void SetVerticalCursor(){_cursor = ImGuiMouseCursor_ResizeNS;};
  void SetDefaultCursor(){_cursor = ImGuiMouseCursor_Arrow;};
  void Resize(int width, int height);
  void Draw();
  
private:
  ImGuiWindowFlags    _flags;
  int*                _pixels;
  unsigned            _viewID;
  unsigned            _width;
  unsigned            _height;
  unsigned            _lastX;
  unsigned            _lastY;
  bool                _drag;
  bool                _valid;
  int                 _cursor;
  std::vector<View*>  _views;
};

AMN_NAMESPACE_CLOSE_SCOPE
