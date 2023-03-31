#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>

#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"
#include "../ui/ui.h"
#include "../ui/tab.h"
#include "../ui/splitter.h"
#include "../ui/menu.h"
#include "../ui/viewport.h"
#include "../ui/contentBrowser.h"
#include "../ui/graphEditor.h"
#include "../ui/propertyEditor.h"
#include "../ui/curveEditor.h"
#include "../ui/layerEditor.h"
#include "../ui/textEditor.h"
#include "../ui/debug.h"
#include "../ui/demo.h"



JVR_NAMESPACE_OPEN_SCOPE
// View constructor
//----------------------------------------------------------------------------
View::View(View* parent, const pxr::GfVec2f& min, const pxr::GfVec2f& max, unsigned flags)
  : _parent(parent)
  , _tab(NULL)
  , _left(NULL)
  , _right(NULL)
  , _min(min)
  , _max(max)
  , _flags(flags)
  , _perc(0.5)
  , _buffered(0)
  , _fixedPixels(-1)
  , _current(NULL)
  , _currentIdx(-1)
{
  if(_parent!=NULL)_window = _parent->_window;
  if (_flags & View::TAB)CreateTab();
}

View::View(View* parent, int x, int y, int w, int h, unsigned flags)
  : _parent(parent)
  , _tab(NULL)
  , _left(NULL)
  , _right(NULL)
  , _min(pxr::GfVec2f(x, y))
  , _max(pxr::GfVec2f(x+w, y+h))
  , _flags(flags)
  , _perc(0.5)
  , _buffered(0)
  , _fixedPixels(-1)
  , _current(NULL)
  , _currentIdx(-1)
{
  if(_parent!=NULL)_window = _parent->_window;
  if (_flags & View::TAB)CreateTab();
}

View::~View()
{
  std::cout << "delete view " << this << std::endl;
  std::cout << _tab << ", " << _left << "," << _right << std::endl;
  if (_tab) delete _tab;
  std::cout << "tab deleted" << std::endl;
  if (_left) delete _left;
  std::cout << "left deleted" << std::endl;
  if (_right) delete _right;
  std::cout << "right deleted" << std::endl;
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


ViewTabUI*
View::CreateTab()
{
  _tab = new ViewTabUI(this); 
  return _tab;
}

void 
View::RemoveTab()
{
  if (_tab)delete _tab;
  _tab = NULL;
}

void
View::CreateUI(UIType type)
{
  switch (type) {
  case UIType::VIEWPORT:
    _current = new ViewportUI(this);
    break;
  case UIType::CONTENTBROWSER:
    _current = new ContentBrowserUI(this);
    break;
  case UIType::GRAPHEDITOR:
    _current = new GraphEditorUI(this);
    break;
  case UIType::CURVEEDITOR:
    _current = new CurveEditorUI(this);
    break;
  case UIType::LAYEREDITOR:
    _current = new LayerEditorUI(this);
    break;
  case UIType::DEMO:
    _current = new DemoUI(this);
    break;
  case UIType::PROPERTYEDITOR:
    _current = new PropertyUI(this);
    break;
  case UIType::TEXTEDITOR:
    _current = new TextEditorUI(this);
    break;
  /*case UIType::DEBUG:
    _current = new DebugUI(this);
    break;*/
  default:
    break;
  }
}

void
View::AddUI(BaseUI* ui)
{
  _uis.push_back(ui);
  _currentIdx = (_uis.size() - 1);
  _current = ui; 
}

void
View::SetCurrentUI(int index)
{
  if (index >= 0 && index < _uis.size())
  {
    _currentIdx = index;
    _current = _uis[index];
    _current->Resize();
  }
}

BaseUI*
View::GetCurrentUI()
{
  return _current;
}

void
View::RemoveUI(int index)
{
  if (index >= 0 && index < _uis.size()) {
    BaseUI* ui = _uis[index];

    _uis.erase(_uis.begin() + index);
  }
}

void
View::RemoveUI(BaseUI* ui)
{
  for (size_t i = 0; i < _uis.size(); ++i) {
    if (_uis[i] == ui) {
      _uis.erase(_uis.begin() + i);
      break;
    }
  }
}


const std::vector<BaseUI*>&
View::GetUIs() const
{
  return _uis;
}

std::vector<BaseUI*>&
View::GetUIs()
{
  return _uis;
}

void
View::TransferUIs(View* source)
{
  _uis = std::move(source->_uis);
  for (auto& ui : _uis)ui->SetParent(this);
  source->_uis.clear();
  if(_uis.size()) {
    _currentIdx = 0;
    _current = _uis[0];
  } else {
    _currentIdx = -1;
    _current = NULL;
  }
}

bool
View::Contains(int x, int y)
{
  if(x>=_min[0] && x<=_max[0] && y>=_min[1] && y<=_max[1])return true;
  else return false;
}

bool
View::Intersect(const pxr::GfVec2i& min, const pxr::GfVec2i& size)
{
  pxr::GfRange2f viewRange(GetMin(), GetMax());
  pxr::GfRange2f boxRange(min, min + size);
  return !boxRange.IsOutside(viewRange);
}

bool
View::DrawTab()
{
  if (_tab) return (_tab->Draw());
  return false;
}

void 
View::Draw(bool forceRedraw)
{
  if (!GetFlag(LEAF)) {
    if (_left)_left->Draw(forceRedraw);
    if (_right)_right->Draw(forceRedraw);
  }
  else {
    if (!DrawTab()) {
      Time& time = GetApplication()->GetTime();
      if (_current && (forceRedraw || GetFlag(INTERACTING) || GetFlag(DIRTY))) {
        if (!_current->Draw() && !IsActive() && !(GetFlag(TIMEVARYING) && time.IsPlaying())) {
          SetClean();
        }
      }
    }
  }
}

bool 
View::IsActive()
{
  return (this == GetWindow()->GetActiveView());
}

bool
View::IsHovered()
{
  return (this == GetWindow()->GetHoveredView());
}

View* View::GetSibling()
{
  if (_parent) {
    if (this == _parent->GetLeft()) {
      return _parent->GetRight();
    }
    else if (this == _parent->GetRight()) {
      return _parent->GetLeft();
    }
  }
  return NULL;
}

void View::DeleteChildren()
{
  if (_left)delete _left;
  if (_right) delete _right;
  _left = _right = NULL;
}

// mouse positon relative to the view
void 
View::GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY)
{
  pxr::GfVec2f position = GetMin();
  int x = position[0];
  int y = position[1];
  outX = inX - x;
  outY = inY - y;
}

float View::GetTabHeight() {
  if (_tab)return _tab->GetHeight();
  else return 0.f;
}

void 
View::MouseButton(int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(GetWindow()->GetGlfwWindow(), &x, &y);
  if (_tab) {
    const float relativeY = y - GetY();
    if (_tab->Invade() || (relativeY > 0 && relativeY < GetTabHeight() * 2)) {
      _tab->MouseButton(button, action, mods);
    } else {
      if (_current && !GetFlag(DISCARDMOUSEBUTTON)) {
        _current->MouseButton(button, action, mods);
      }
      ClearFlag(DISCARDMOUSEBUTTON);
    }
  } else {
    if (_current && !GetFlag(DISCARDMOUSEBUTTON)) {
      _current->MouseButton(button, action, mods);
    }
    ClearFlag(DISCARDMOUSEBUTTON);
  }
}

void 
View::MouseMove(int x, int y)
{
  if (_tab) {
    if ((y - GetY()) < GetTabHeight()) {
      if (GetFlag(View::INTERACTING) && _current)_current->MouseMove(x, y);
      else _tab->MouseMove(x, y);
    } else {
      if (_current && !GetFlag(DISCARDMOUSEMOVE))
        _current->MouseMove(x, y);
        ClearFlag(DISCARDMOUSEMOVE);
    }
  } else {
    if (_current && !GetFlag(DISCARDMOUSEMOVE))
      _current->MouseMove(x, y);
    ClearFlag(DISCARDMOUSEMOVE);
  }
}

void 
View::MouseWheel(int x, int y)
{
  if(_current)_current->MouseWheel(x, y);
}

void 
View::Keyboard(int key, int scancode, int action, int mods)
{
  if (_current) {
    _current->Keyboard(key, scancode, action, mods);
  }
}

void 
View::Input(int key)
{
  if (_current) {
    _current->Input(key);
  }
}

void
View::Focus(int state)
{
  if (state)SetFlag(FOCUS);
  else ClearFlag(FOCUS);
  if (_tab)_tab->Focus(state);
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
  _left->_parent = this;

  GetChildMinMax(false, cMin, cMax);
  _right = new View(this, cMin, cMax);
  _right->_parent = this;

  ComputeNumPixels(false);
  ClearFlag(LEAF);

  if (_tab && _left->_tab) {
    _left->TransferUIs(this);
  }
  RemoveTab();
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
    if(_current)_current->Resize();
  }
  if(rationalize)RescaleNumPixels(ratio);
  SetDirty();
  
  _window->ForceRedraw();
  
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

void
View::SetClean()
{
  if (_buffered <= 0) {
    ClearFlag(DIRTY);
  }
  else _buffered--;
}

void
View::SetDirty()
{
  SetFlag(DIRTY);
  _buffered = 3;
}

void 
View::SetTabed(bool tabed)
{
  if (tabed) {
    if (!_tab) _tab = CreateTab();
  }
  else {
    if (_tab) RemoveTab();
  }
}

void 
View::SetInteracting(bool value) 
{
  if (value) SetFlag(INTERACTING);
  else ClearFlag(INTERACTING);
}

bool 
View::IsInteracting()
{
  return GetFlag(INTERACTING);
}


JVR_NAMESPACE_CLOSE_SCOPE