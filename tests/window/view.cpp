#include "view.h"
#include "window.h"
#include "ui.h"
#include "splitter.h"
#include "../imgui/imgui_test.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>


JVR_NAMESPACE_OPEN_SCOPE

// View constructor
//----------------------------------------------------------------------------
View::View(View* parent, const pxr::GfVec2f& min, const pxr::GfVec2f& max):
  _parent(parent), 
  _left(NULL),
  _right(NULL),
  _min(min), 
  _max(max), 
  _flags(HORIZONTAL|LEAF),
  _perc(0.5),
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
  _left(NULL),
  _right(NULL),
  _min(pxr::GfVec2f(x, y)), 
  _max(pxr::GfVec2f(x+w, y+h)), 
  _flags(HORIZONTAL|LEAF),
  _perc(0.5),
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
  if(_content)delete _content;
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

void View::SetContent(BaseUI* ui)
{
  if(_content)delete _content; 
  _content=ui;
};

bool
View::Contains(int x, int y)
{
  if(x>=_min[0] && x<=_max[0] && y>=_min[1] && y<=_max[1])return true;
  else return false;
}

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
}

// mouse positon relative to the view
void View::GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY)
{
  pxr::GfVec2f position =GetMin();
  int x = position[0];
  int y = position[1];
  outX = inX - x;
  outY = inY - y;
}

void 
View::MouseButton(int action, int button, int mods)
{
  if(_content)_content->MouseButton(action, button, mods);
}

void 
View::MouseMove(int x, int y)
{
  if(_content)_content->MouseMove(x, y);
}

void 
View::MouseWheel(int x, int y)
{
  if(_content)_content->MouseWheel(x, y);
}

void
View::GetChildMinMax(bool leftOrRight, pxr::GfVec2f& cMin, pxr::GfVec2f& cMax)
{
  // horizontal splitter
  if(IsHorizontal())
  {
    if(leftOrRight)
    {
      cMin = GetMin();
      cMax[0] = GetMax()[0];
      cMax[1] = GetMin()[1] + (GetMax()[1]-GetMin()[1]) * _perc;
    }
    else
    { 
      cMin[0] = GetMin()[0];
      cMin[1] = GetMin()[1] + (GetMax()[1]-GetMin()[1]) * _perc;
      cMax = GetMax();
    }
  }
  // vertical splitter
  else
  {
    if(leftOrRight)
    {
      cMin = GetMin();
      cMax[0] = GetMin()[0] + (GetMax()[0]-GetMin()[0]) * _perc;
      cMax[1] = GetMax()[1];
    }
    else
    {
      cMin[0] = GetMin()[0] + (GetMax()[0]-GetMin()[0]) * _perc;
      cMin[1] = GetMin()[1];
      cMax = GetMax();
    }
  }
}

void
View::GetSplitInfos(pxr::GfVec2f& sMin, pxr::GfVec2f& sMax,
  const int width, const int height)
{
  if(IsHorizontal())
  { 
    sMin[0] = GetMin()[0];
    sMax[0] = GetMax()[0];
  
    int h = GetMin()[1] + (GetMax()[1] - GetMin()[1]) * (GetPerc());
    sMin[1] = h - SPLITTER_THICKNESS;
    sMax[1] = h + SPLITTER_THICKNESS;
    sMin[1] = (sMin[1] < 0) ? 0 : ((sMin[1] > height) ? height : sMin[1]);
    sMax[1] = (sMax[1] < 0) ? 0 : ((sMax[1] > height) ? height : sMax[1]);
  }
  else
  {
    int w = GetMin()[0] + (GetMax()[0]-GetMin()[0]) * (GetPerc());
    sMin[0] = w - SPLITTER_THICKNESS;
    sMax[0] = w + SPLITTER_THICKNESS;
    sMin[1] = GetMin()[1];
    sMax[1] = GetMax()[1];
    sMin[1] = (sMin[1] < 0) ? 0 : ((sMin[1] > width) ? width : sMin[1]);
    sMax[1]= (sMax[1] < 0) ? 0 : ((sMax[1] > width) ? width : sMax[1]);
  }
}

void
View::Split(double perc, bool horizontal, bool fixed)
{
  if(horizontal)SetHorizontal();
  else ClearHorizontal();

  if(fixed)SetFixed();
  else ClearFixed();
  _perc = perc;

  pxr::GfVec2f cMin, cMax;    
  GetChildMinMax(true, cMin, cMax);
  _left = new View(this, cMin, cMax);
  _left->_name = _name + ":left";
  _left->_parent = this;

  GetChildMinMax(false, cMin, cMax);
  _right = new View(this, cMin, cMax);
  _right->_name = _name + ":right";
  _right->_parent = this;

  ClearLeaf();
}

void 
View::Resize(int x, int y, int w, int h, bool rationalize)
{
  pxr::GfVec2f ratio;
  if(rationalize)
  {
    ratio[0] = 1 / ((double)(_max[0] - _min[0]) / (double)w);
    ratio[1] = 1 / ((double)(_max[1] - _min[1]) / (double)h);
  }
  _min = pxr::GfVec2f(x, y);
  _max = pxr::GfVec2f(x+w, y+h);
  
  if(!IsLeaf())
  {
    if(IsHorizontal())
    {
      if(_left)_left->Resize(x, y, w, h * _perc);
      if(_right)_right->Resize(x, y + h * _perc, w, h - h *_perc);
    }
    else
    {
      if(_left)_left->Resize(x, y, w * _perc, h);
      if(_right)_right->Resize(x + w * _perc, y, w - w * _perc, h);
    }
  }
  else
  {
    if(_content)_content->Resize();
  }
  if(rationalize)RescaleNumPixels(ratio);
}

void 
View::GetPercFromMousePosition(int x, int y)
{
  if(IsHorizontal())
  {
    if(GetHeight()<0.01)return;
    double perc = (double)(y - _min[1]) / (double)(_max[1] - _min[1]);
    _perc = perc < 0.05 ? 0.05 : perc > 0.95 ? 0.95 : perc;
  }
  else
  {
    if(GetWidth()<0.01)return;
    double perc = (double)(x - _min[0]) / (double)(_max[0] - _min[0]);
    _perc = perc < 0.05 ? 0.05 : perc > 0.95 ? 0.95 : perc;
  }
  ComputeNumPixels(true);
 
}

void 
View::SetPerc(double perc)
{
  _perc=perc;
  ComputeNumPixels(false);
};

void
View::ComputeNumPixels(bool postFix)
{
  if(IsHorizontal())
  {
    _npixels[0] = _perc * GetHeight();
    _npixels[1] = (1-_perc) * GetHeight();
  }
  else
  {
    _npixels[0] = _perc * GetWidth();
    _npixels[1] = (1-_perc) * GetWidth();
  }
  
  if(postFix)
  {
    if(_left)FixLeft();
    if(_right)FixRight();
  }
}
void 
View::RescaleNumPixels(pxr::GfVec2f  ratio)
{
  if(!IsLeaf())
  {
    if(IsHorizontal()){_npixels[0] *= ratio[1]; _npixels[1] *= ratio[1];}
    else {_npixels[0] *= ratio[0]; _npixels[1] *= ratio[0];}

    if(_left)_left->RescaleNumPixels(ratio);
    if(_right)_right->RescaleNumPixels(ratio);
  }
}


void 
View::FixLeft()
{
  if(_left->IsHorizontal() == IsHorizontal())
  {
    double perc;
    if(IsHorizontal())
    {
      int height = GetHeight() * _perc;
      perc = (double)_left->_npixels[0] / (double)height;
    }
    else
    { 
      double width = GetWidth() * _perc;
      perc = (double)_left->_npixels[0] / (double)width;
    }

     _left->_perc = perc < 0.05 ? 0.05 : perc > 0.95 ? 0.95 : perc;
      
    if(!_left->IsLeaf())
    {
      _left->FixLeft();
      _left->FixRight();
    }
  }
}

void 
View::FixRight()
{
  if(_right->IsHorizontal() == IsHorizontal())
  {
    double perc;
    if(IsHorizontal())
    {
      double height = GetHeight() * (1 - _perc);
      perc = 1-(double)_right->_npixels[1] / (double)height;
    }
    else
    { 
      double width = GetWidth() * (1.0 - _perc);
      perc = 1.0 - (double)_right->_npixels[1] / (double)width;
    }
    _right->_perc = perc < 0.05 ? 0.05 : perc > 0.95 ? 0.95 : perc;

    if(!_right->IsLeaf())
    {
      _right->FixLeft();
      _right->FixRight();
    }
  }
  
}

JVR_NAMESPACE_CLOSE_SCOPE