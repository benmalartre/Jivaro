#include "view.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>
namespace AMN {

  // view constructor
  //----------------------------------------------------------------------------
  View::View(View* parent, const pxr::GfVec2i& min, const pxr::GfVec2i& max):
    _parent(parent), 
    _min(min), 
    _max(max), 
    _flags(HORIZONTAL|LEAF)
  {
    _color = pxr::GfVec3f(
      RANDOM_0_1,
      RANDOM_0_1,
      RANDOM_0_1
    );
  }

  View::View(View* parent, int x, int y, int w, int h):
    _parent(parent), 
    _min(pxr::GfVec2i(x, y)), 
    _max(pxr::GfVec2i(x+w, y+h)), 
    _flags(HORIZONTAL|LEAF)
  {
    _color = pxr::GfVec3f(
      RANDOM_0_1,
      RANDOM_0_1,
      RANDOM_0_1
    );
  }

  void 
  View::Draw()
  {
    int x, y, w, h;
    if(!IsHorizontal())
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
  View::MouseEnter()
  {
    _color = pxr::GfVec3f(1.f, 0.f, 0.f);
  }

  void 
  View::MouseLeave()
  {
    _color = pxr::GfVec3f(0.f, 1.f, 0.f);
  }

  void
  View::GetChildMinMax(bool leftOrRight, pxr::GfVec2i& cMin, pxr::GfVec2i& cMax)
  {
    // horizontal splitter
    if(IsHorizontal())
    {
      if(leftOrRight)
      {
        cMin = GetMin();
        cMax = GetMin() + 
          GfCompMult(GetMax()-GetMin(), pxr::GfVec2i(1.f, _perc / 100));
      }
      else
      {
        cMin = GetMin() + 
          GfCompMult(GetMax()-GetMin(), pxr::GfVec2i(1.f, _perc / 100));
        cMax = GetMax();
      }
    }
    // vertical splitter
    else
    {
      if(leftOrRight)
      {
        cMin = GetMin();
        cMax = GetMin() + 
          GfCompMult(GetMax()-GetMin(), pxr::GfVec2i(_perc / 100, 1.f));
      }
      else
      {
        cMin = GetMin() + 
          GfCompMult(GetMax()-GetMin(), pxr::GfVec2i(_perc / 100, 1.f));
        cMax = GetMax();
      }
    }
  }

  void
  View::Split()
  {
    pxr::GfVec2i cMin, cMax;
    GetChildMinMax(true, cMin, cMax);
    _left = new View(this, cMin, cMax);
    GetChildMinMax(false, cMin, cMax);
    _right = new View(this, cMin, cMax);

    if(_content){
      std::cerr << 
        "WE GOT FUCKIN CONTENT : WE HAVE TO MOVE THIS SHIT FROM HERE !!!! " 
          << std::endl;
    }
  }

  void 
  ViewMap::AddView(View* splitter)
  {

  }

  void 
  ViewMap::BuildMap(int width, int height)
  {
    if(_pixels)delete [] _pixels;
    if(_window)
    {
      _pixels = new char[width * height];
      for(auto item: _map)
      {
        const pxr::GfVec2i smin(0,0);
        //for(int x= splitter.second.GetMin()[0];
      }
    }
    
  }


} // namespace AMN