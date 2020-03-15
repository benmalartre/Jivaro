#pragma once

#include "../default.h"
#include "pxr/pxr.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>

AMN_NAMESPACE_OPEN_SCOPE
class AmnView;
class AmnWindow;

enum BORDER
{
  TOP     = 1,
  RIGHT   = 2,
  BOTTOM  = 4,
  LEFT    = 8
};

#define SPLITTER_THICKNESS 2

class AmnSplitter
{
public: 
  AmnSplitter():_pixels(NULL){};
  ~AmnSplitter(){
    std::cout << "DELETE SPLITTER PIXEL BEFORE" << std::endl;
    if(_pixels)delete [] _pixels;
    std::cout << "DELETE SPLITTER PIXEL AFTER" << std::endl;
  };

  int* GetPixels(){return _pixels;};
  inline unsigned GetWidth(){return _width;};
  inline unsigned GetHeight(){return _height;};
  void RecurseBuildMap(AmnView* view);
  void BuildMap(AmnView* view);
  AmnView* GetViewByIndex(int index);
  int GetPixelValue(double xPos, double yPos);

  void Resize(int width, int height, AmnView* view);
  void Draw();
  void Event();
  
private:
  int*                _pixels;
  unsigned            _viewID;
  unsigned            _width;
  unsigned            _height;
  unsigned            _lastX;
  unsigned            _lastY;
  bool                _drag;
  std::vector<AmnView*>  _views;
};

AMN_NAMESPACE_CLOSE_SCOPE

