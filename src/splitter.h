#pragma once

#include "default.h"
#include "view.h"
#include "pxr/pxr.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>


namespace AMN {
  class View;
  class Window;

  class Splitter
  {
  public:
    Splitter(Window* window, const pxr::GfVec2i& min, const pxr::GfVec2i& max);
    Splitter(Window* window, int x, int y, int w, int h, int perc);
    const pxr::GfVec2i& GetMin(){return _min;};
    const pxr::GfVec2i& GetMax(){return _max;};
    int GetPerc(){return _perc;};
    const pxr::GfVec3f& GetColor(){return _color;};
    void Draw();
    void MouseEnter();
    void MouseLeave();
  private:
    pxr::GfVec2i      _min;
    pxr::GfVec2i      _max;
    pxr::GfVec3f      _color;
    int               _perc;
    View*             _left;
    View*             _right;
    Window*           _window;
    bool              _over;
    bool              _horizontal;
  };

  class SplitterMap
  {
  public: 
    SplitterMap():_pixels(NULL){};
    ~SplitterMap(){if(_pixels)delete [] _pixels;};

    void AddSplitter(Splitter* splitter);
    void BuildMap();
    
  private:
    Window* _window;
    const char* _pixels;
    std::map<char, Splitter*> _map;
  };

} // namespace AMN
