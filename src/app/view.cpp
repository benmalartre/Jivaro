#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../ui/ui.h"
#include "../ui/head.h"
#include "../ui/splitter.h"
#include "../ui/menu.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>


PXR_NAMESPACE_OPEN_SCOPE

// View constructor
//----------------------------------------------------------------------------
View::View(View* parent, const pxr::GfVec2f& min, const pxr::GfVec2f& max)
  : _parent(parent)
  , _head(NULL)
  , _left(NULL)
  , _right(NULL)
  , _min(min)
  , _max(max)
  , _flags(HORIZONTAL|LEAF|DIRTY)
  , _perc(0.5)
  , _content(NULL)
  , _buffered(0)
  , _fixedPixels(-1)
{
  if(_parent==NULL)_name = "main";
  else _window = _parent->_window;
}

View::View(View* parent, int x, int y, int w, int h)
  : _parent(parent)
  , _head(NULL)
  , _left(NULL)
  , _right(NULL)
  , _min(pxr::GfVec2f(x, y))
  , _max(pxr::GfVec2f(x+w, y+h))
  , _flags(HORIZONTAL|LEAF|DIRTY)
  , _perc(0.5)
  , _content(NULL)
  , _buffered(0)
  , _fixedPixels(-1)
{
  if(_parent==NULL)_name = "main";
  else _window = _parent->_window;
}

View::~View()
{
  if (_head) delete _head;
  if (_content) delete _content;
  if (_left) delete _left;
  if (_right) delete _right;
}

void 
View::SetWindow(Window* window)
{
  _window = window;
}

Window* 
View::GetWindow()
{
  return _window;
}

void 
View::SetContent(BaseUI* ui)
{
  _content=ui;
};

ViewHead*
View::CreateHead()
{
  _head = new ViewHead(this); 
  return _head;
}

bool
View::Contains(int x, int y)
{
  if(x>=_min[0] && x<=_max[0] && y>=_min[1] && y<=_max[1])return true;
  else return false;
}

void
View::DrawHead()
{
  if (_head) _head->Draw();
}

void 
View::Draw(bool forceRedraw)
{
  if (!GetFlag(LEAF)) {
    if (_left)_left->Draw(forceRedraw);
    if (_right)_right->Draw(forceRedraw);
  }
  else {
    bool bForceRedraw = GetFlag(FORCEREDRAW) ? true : forceRedraw;
    DrawHead();
    if (_content && (bForceRedraw || GetFlag(INTERACTING) || GetFlag(DIRTY))) {
      if (!_content->Draw() && !bForceRedraw) {
        SetClean();
      }
    }
  }
}

// mouse positon relative to the view
void 
View::GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY)
{
  pxr::GfVec2f position =GetMin();
  int x = position[0];
  int y = position[1];
  outX = inX - x;
  outY = inY - y;
}

void 
View::MouseButton(int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(GetWindow()->GetGlfwWindow(), &x, &y);
  if (_head) {
    const float relativeY = y - GetY();
    if (relativeY > 0 && relativeY < VIEW_HEAD_HEIGHT) {
      _head->MouseButton(button, action, mods);
    } else {
      if (_content && !GetFlag(DISCARDMOUSEBUTTON))
        _content->MouseButton(button, action, mods);
      ClearFlag(DISCARDMOUSEBUTTON);
    }
  } else {
    if (_content && !GetFlag(DISCARDMOUSEBUTTON))
      _content->MouseButton(button, action, mods);
    ClearFlag(DISCARDMOUSEBUTTON);
  }
}

void 
View::MouseMove(int x, int y)
{
  if (_head) {
    if ((y - GetY()) < VIEW_HEAD_HEIGHT) {
      if (GetFlag(View::INTERACTING) && _content)_content->MouseMove(x, y);
      else _head->MouseMove(x, y);
    } else {
      if (_content)_content->MouseMove(x, y);
    }
  } else {
    if (_content)_content->MouseMove(x, y);
  }
}

void 
View::MouseWheel(int x, int y)
{
  if(_content)_content->MouseWheel(x, y);
}

void 
View::Keyboard(int key, int scancode, int action, int mods)
{
  if (_content) {
    _content->Keyboard(key, scancode, action, mods);
  }
}

void
View::GetChildMinMax(bool leftOrRight, pxr::GfVec2f& cMin, pxr::GfVec2f& cMax)
{
  // horizontal splitter
  if(GetFlag(HORIZONTAL)) {
    if (leftOrRight) {
      cMin = GetMin();
      cMax[0] = GetMax()[0];
      cMax[1] = GetMin()[1] + (GetMax()[1] - GetMin()[1]) * _perc;
    }
    else {
      cMin[0] = GetMin()[0];
      cMin[1] = GetMin()[1] + (GetMax()[1] - GetMin()[1]) * _perc;
      cMax = GetMax();
    }
  }
  // vertical splitter
  else
  {
    if (leftOrRight)
    {
      cMin = GetMin();
      cMax[0] = GetMin()[0] + (GetMax()[0] - GetMin()[0]) * _perc;
      cMax[1] = GetMax()[1];
    }
    else
    {
      cMin[0] = GetMin()[0] + (GetMax()[0] - GetMin()[0]) * _perc;
      cMin[1] = GetMin()[1];
      cMax = GetMax();
    }
  }
}

void
View::GetSplitInfos(pxr::GfVec2f& sMin, pxr::GfVec2f& sMax,
  const int width, const int height)
{
  if(GetFlag(HORIZONTAL))
  { 
    sMin[0] = GetMin()[0] - SPLITTER_THICKNESS;
    sMax[0] = GetMax()[0] + SPLITTER_THICKNESS;
  
    int h = GetMin()[1] + (GetMax()[1] - GetMin()[1]) * GetPerc();
    sMin[1] = h - SPLITTER_THICKNESS;
    sMax[1] = h + SPLITTER_THICKNESS;
    sMin[1] = (sMin[1] < 0) ? 0 : ((sMin[1] > height) ? height : sMin[1]);
    sMax[1] = (sMax[1] < 0) ? 0 : ((sMax[1] > height) ? height : sMax[1]);
  }
  else
  {
    int w = GetMin()[0] + (GetMax()[0] - GetMin()[0]) * GetPerc();
    sMin[0] = w - SPLITTER_THICKNESS;
    sMax[0] = w + SPLITTER_THICKNESS;
    sMin[1] = GetMin()[1] - SPLITTER_THICKNESS;
    sMax[1] = GetMax()[1] + SPLITTER_THICKNESS;
    sMin[1] = (sMin[1] < 0) ? 0 : ((sMin[1] > width) ? width : sMin[1]);
    sMax[1]= (sMax[1] < 0) ? 0 : ((sMax[1] > width) ? width : sMax[1]);
  }
}

void
View::Split(double perc, bool horizontal, int fixed, int numPixels)
{
  if(horizontal)SetFlag(HORIZONTAL);
  else ClearFlag(HORIZONTAL);

  if (fixed && numPixels) {
    if (fixed & LFIXED) {
      SetFlag(LFIXED); 
      ClearFlag(RFIXED);
    }
    else if (fixed & RFIXED) {
      SetFlag(RFIXED);
      ClearFlag(LFIXED);
    }
  }
  else {
    ClearFlag(LFIXED);
    ClearFlag(RFIXED);
  }
  _perc = perc;
  _fixedPixels = numPixels;

  pxr::GfVec2f cMin, cMax;    
  GetChildMinMax(true, cMin, cMax);
  _left = new View(this, cMin, cMax);
  _left->_name = _name + ":left";
  _left->_parent = this;

  GetChildMinMax(false, cMin, cMax);
  _right = new View(this, cMin, cMax);
  _right->_name = _name + ":right";
  _right->_parent = this;

  ComputeNumPixels(false);
  ClearFlag(LEAF);
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
  _min = pxr::GfVec2f(x , y);
  _max = pxr::GfVec2f(x + w, y + h);
  
  if(!GetFlag(LEAF))
  {
    if(GetFlag(HORIZONTAL))
    {
      if (GetFlag(LFIXED)) _perc = (double)_fixedPixels / (double)h;
      else if (GetFlag(RFIXED)) _perc = (double)(h - _fixedPixels) / (double)h;
      
      double ph = (double)h * _perc;
      if (_left)_left->Resize(x, y, w, ph);
      if (_right)_right->Resize(x, y + ph, w, h - ph);
    }
    else
    {
      if (GetFlag(LFIXED)) _perc = (float)_fixedPixels / (float)w;
      else if (GetFlag(RFIXED)) _perc = (float)(w - _fixedPixels) / (float)w;
      
      double pw = (double)w * _perc;
      if (_left)_left->Resize(x, y, pw, h);
      if (_right)_right->Resize(x + pw, y, w - pw, h);
    }
  }
  else
  {
    if(_content)_content->Resize();
  }
  if(rationalize)RescaleNumPixels(ratio);
  SetDirty();
  Application* app = GetApplication();
}

void 
View::GetPercFromMousePosition(int x, int y)
{
  if(GetFlag(HORIZONTAL))
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
  float side = GetFlag(HORIZONTAL) ? GetHeight() : GetWidth();

  if (GetFlag(LFIXED)) {
    _numPixels[0] = _fixedPixels;
    _numPixels[1] = side - _fixedPixels;
  }
  else if (GetFlag(RFIXED)) {
    _numPixels[0] = side - _fixedPixels;
    _numPixels[1] = _fixedPixels;
  }
  else {
    _numPixels[0] = _perc * side;
    _numPixels[1] = (1 - _perc) * side;
  }

  if(postFix)
  {
    if(_left)RescaleLeft();
    if(_right)RescaleRight();
  }
}

void 
View::RescaleNumPixels(pxr::GfVec2f  ratio)
{
  if(!GetFlag(LEAF))
  {
    if(GetFlag(HORIZONTAL)){_numPixels[0] *= ratio[1]; _numPixels[1] *= ratio[1];}
    else {_numPixels[0] *= ratio[0]; _numPixels[1] *= ratio[0];}

    if(_left)_left->RescaleNumPixels(ratio);
    if(_right)_right->RescaleNumPixels(ratio);
  }
}

void 
View::RescaleLeft()
{
  if(_left->GetFlag(HORIZONTAL) == GetFlag(HORIZONTAL))
  {
    double perc;
    if(GetFlag(HORIZONTAL)) {
      double height = GetHeight() * _perc;
      perc = (double)_left->_numPixels[0] / height;
    }
    else { 
      double width = GetWidth() * _perc;
      perc = (double)_left->_numPixels[0] / width;
    }

     _left->_perc = perc < 0.05 ? 0.05 : perc > 0.95 ? 0.95 : perc;
      
    if(!_left->GetFlag(LEAF))
    {
      _left->RescaleLeft();
      _left->RescaleRight();
    }
  }
}

void 
View::RescaleRight()
{
  if(_right->GetFlag(HORIZONTAL) == GetFlag(HORIZONTAL))
  {
    double perc;
    if(GetFlag(HORIZONTAL)) {
      double height = GetHeight() * (1.0 - _perc);
      perc = 1.0-(double)_right->_numPixels[1] / (double)height;
    }
    else { 
      double width = GetWidth() * (1.0 - _perc);
      perc = 1.0 - (double)_right->_numPixels[1] / (double)width;
    }
    _right->_perc = perc < 0.05 ? 0.05 : perc > 0.95 ? 0.95 : perc;

    if(!_right->GetFlag(LEAF))
    {
      _right->RescaleLeft();
      _right->RescaleRight();
    }
  }
}

void View::SetClean()
{
  if (_buffered >= 3) {
    ClearFlag(DIRTY);
    _buffered = 0;
  }
  else _buffered++;
}

void View::SetDirty()
{
  SetFlag(DIRTY);
  _buffered = 0;
}

void View::SetInteracting(bool value) 
{
  if (value) SetFlag(INTERACTING);
  else ClearFlag(INTERACTING);
}

bool View::IsInteracting()
{
  return GetFlag(INTERACTING);
}


PXR_NAMESPACE_CLOSE_SCOPE