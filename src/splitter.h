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

  #define SPLITTER_THICKNESS 4

  class Splitter
  {
  public: 
    Splitter():_pixels(NULL){};
    ~Splitter(){if(_pixels)delete [] _pixels;};

    unsigned* GetPixels(){return _pixels;};
    void BuildMap(View* view);
    void RecurseBuildMap(View* view);
    
  private:
    Window* _window;
    unsigned* _pixels;
    std::map<char, View*> _map;
    unsigned _viewID;
  };

} // namespace AMN
