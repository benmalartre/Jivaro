#include "view.h"
#include "window.h"
#include "ui.h"
#include "splitter.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>
#include "imgui/imgui_test.h"

namespace AMN {

  // view constructor
  //----------------------------------------------------------------------------
  View::View(View* parent, const pxr::GfVec2i& min, const pxr::GfVec2i& max):
    _parent(parent), 
    _min(min), 
    _max(max), 
    _flags(HORIZONTAL|LEAF),
    _perc(50),
    _content(NULL)
  {
    if(_parent==NULL)_name = "main";
    else _window = _parent->_window;
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
    _flags(HORIZONTAL|LEAF),
    _perc(50),
    _content(NULL)
  {
    if(_parent==NULL)_name = "main";
    else _window = _parent->_window;
    _color = pxr::GfVec3f(
      RANDOM_0_1,
      RANDOM_0_1,
      RANDOM_0_1
    );
  }

  View::~View()
  {
    if(_left)delete _left;
    if(_right)delete _right;
  }

  void View::SetWindow(Window* window)
  {
    _window = window;
  }

  Window* View::GetWindow()
  {
    return _window;
  }

  void View::SetContent(UI* ui)
  {
    if(_content)delete _content; 
    _content=ui;
  };

  void 
  View::Draw()
  {
    if(!IsLeaf()){
      if(_left)_left->Draw();
      if(_right)_right->Draw();
    }
    else
    {
      if(_content)_content->Draw();
    }
    
   
    
    /*
    int x, y, w, h;
    if(!BITMASK_CHECK(_flags, HORIZONTAL))
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
    */
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
        cMax[0] = GetMax()[0];
        cMax[1] = GetMin()[1] + (GetMax()[1]-GetMin()[1]) * _perc * 0.01f;
      }
      else
      { 
        cMin[0] = GetMin()[0];
        cMin[1] = GetMin()[1] + (GetMax()[1]-GetMin()[1]) * _perc * 0.01f;
        cMax = GetMax();
      }
    }
    // vertical splitter
    else
    {
      if(leftOrRight)
      {
        cMin = GetMin();
        cMax[0] = GetMin()[0] + (GetMax()[0]-GetMin()[0]) * _perc * 0.01f;
        cMax[1] = GetMax()[1];
      }
      else
      {
        cMin[0] = GetMin()[0] + (GetMax()[0]-GetMin()[0]) * _perc * 0.01f;
        cMin[1] = GetMin()[1];
        cMax = GetMax();
      }
    }
  }

  void
  View::GetSplitInfos(pxr::GfVec2i& sMin, pxr::GfVec2i& sMax,
    const int width, const int height)
  {
    if(IsHorizontal())
    { 
      sMin[0] = GetMin()[0];
      sMax[0] = GetMax()[0];
    
      int h = GetMin()[1] + (GetMax()[1] - GetMin()[1]) * (GetPerc() * 0.01f);
      sMin[1] = h - SPLITTER_THICKNESS;
      sMax[1] = h + SPLITTER_THICKNESS;
      sMin[1] = (sMin[1] < 0) ? 0 : ((sMin[1] > height) ? height : sMin[1]);
      sMax[1] = (sMax[1] < 0) ? 0 : ((sMax[1] > height) ? height : sMax[1]);
    }
    else
    {
      int w = GetMin()[0] + (GetMax()[0]-GetMin()[0]) * (GetPerc() * 0.01f);
      sMin[0] = w - SPLITTER_THICKNESS;
      sMax[0] = w + SPLITTER_THICKNESS;
      sMin[1] = GetMin()[1];
      sMax[1] = GetMax()[1];
      sMin[1] = (sMin[1] < 0) ? 0 : ((sMin[1] > width) ? width : sMin[1]);
      sMax[1]= (sMax[1] < 0) ? 0 : ((sMax[1] > width) ? width : sMax[1]);
    }
  }

  void
  View::Split()
  {
    pxr::GfVec2i cMin, cMax;    
    GetChildMinMax(true, cMin, cMax);
    _left = new View(this, cMin, cMax);
    _left->_name = _name + ":left";
    _left->_parent = this;

    GetChildMinMax(false, cMin, cMax);
    _right = new View(this, cMin, cMax);
    _right->_name = _name + ":right";
    _right->_parent = this;

    if(_content){
      std::cerr << 
        "WE GOT FUCKIN CONTENT : WE HAVE TO MOVE THIS SHIT FROM HERE !!!! " 
          << std::endl;
    }

    ClearLeaf();
  }

  void 
  View::Resize(int x, int y, int w, int h)
  {
    _min = pxr::GfVec2i(x, y);
    _max = pxr::GfVec2i(x+w, y+h);
    if(!IsLeaf())
    {
      if(IsHorizontal())
      {
        if(_left)_left->Resize(x, y, w, h * _perc * 0.01f);
        if(_right)_right->Resize(x, y + h * _perc * 0.01f, w, h - h *_perc * 0.01f);
      }
      else
      {
        if(_left)_left->Resize(x, y, w * _perc * 0.01f, h);
        if(_right)_right->Resize(x + w * _perc * 0.01f, y, w - w * _perc * 0.01f, h);
      }

    }
  }

  int 
  View::GetPercFromMousePosition(int x, int y)
  {
    if(IsHorizontal())
    {
      float perc = (float)(y - _min[1]) / (float)(_max[1] - _min[1]);
      perc = perc < 0.02f ? 0.02f : perc > 0.98f ? 0.98f : perc;
      _perc = (int)(perc * 100);
    }
    else
    {
      float perc = (float)(x - _min[0]) / (float)(_max[0] - _min[0]);
      perc = perc < 0.02f ? 0.02f : perc > 0.98f ? 0.98f : perc;
      _perc = (int)(perc * 100);
    }
  }

} // namespace AMN