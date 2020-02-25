#include "splitter.h"
#include "window.h"
namespace AMN {

  // splitter constructor
  //----------------------------------------------------------------------------
  Splitter::Splitter(Window* window, 
    const pxr::GfVec2i& min, const pxr::GfVec2i& max):
      _window(window), _min(min), _max(max), _perc(50), _horizontal(true)
  {
  }

  Splitter::Splitter(Window* window, int x, int y, int w, int h, int perc):
    _window(window), _min(pxr::GfVec2i(x, y)),
    _max(pxr::GfVec2i(x+w, y+h)), _perc(perc), _horizontal(true)
  {
  }

  void 
  Splitter::Draw()
  {
    int x, y, w, h;
    if(!_horizontal)
    {
      x = (_min[0] + ((_max[0]-_min[0]) * _perc)/100) - 1;
      y = _min[1];
      w = 2;
      h = _max[1] - _min[1];
    }
    else
    {
      x = _min[0];
      y = (_min[1] + ((_max[1]-_min[1]) * _perc)/100 ) - 1;
      w = _max[0] - _min[0];
      h = 2;
    }
    
    
    glScissor(x,y,w,h);
    //glClearColor(_color[0], _color[1], _color[2], 1.f);
    glClearColor(
      (float)rand()/(float)RAND_MAX,
      (float)rand()/(float)RAND_MAX,
      (float)rand()/(float)RAND_MAX,
      1.f
    );
    glClear(GL_COLOR_BUFFER_BIT);

  }

  void 
  Splitter::MouseEnter()
  {
    _color = pxr::GfVec3f(1.f, 0.f, 0.f);
  }

  void 
  Splitter::MouseLeave()
  {
    _color = pxr::GfVec3f(0.f, 1.f, 0.f);
  }

  void 
  SplitterMap::AddSplitter(Splitter* splitter)
  {

  }

  void 
  SplitterMap::BuildMap()
  {
    if(_pixels)delete [] _pixels;
    if(_window)
    {
      _pixels = new char[_window->GetWidth() * _window->GetHeight()];
      for(auto item: _map)
      {
        const pxr::GfVec2i smin(0,0);
        //for(int x= splitter.second.GetMin()[0];
      }
    }
    
  }


} // namespace AMN