#pragma once

#include "default.h"
#include "pxr/pxr.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>

namespace AMN {
  class View;
  class Window;

  enum BORDER
  {
    TOP     = 1,
    RIGHT   = 2,
    BOTTOM  = 4,
    LEFT    = 8
  };

  #define SPLITTER_THICKNESS 2

  class Splitter
  {
  public: 
    Splitter():_pixels(NULL){};
    ~Splitter(){if(_pixels)delete [] _pixels;};

    unsigned* GetPixels(){return _pixels;};
    inline unsigned GetWidth(){return _width;};
    inline unsigned GetHeight(){return _height;};
    void RecurseBuildMap(View* view);
    void BuildMap(View* view);
    View* GetViewByIndex(int index);
    unsigned GetPixelValue(double xPos, double yPos);

    void Resize(View* view);
    void Draw();
    void Event();
    
  private:
    unsigned*           _pixels;
    unsigned            _viewID;
    unsigned            _width;
    unsigned            _height;
    unsigned            _lastX;
    unsigned            _lastY;
    bool                _drag;
    std::vector<View*>  _views;
  };

} // namespace AMN
