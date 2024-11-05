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
#include "../ui/attributeEditor.h"
#include "../ui/curveEditor.h"
#include "../ui/debug.h"
#include "../ui/demo.h"
#include "../ui/icon.h"
#include "../ui/tool.h"


JVR_NAMESPACE_OPEN_SCOPE
// View constructor
//----------------------------------------------------------------------------
View::View(View* parent, const GfVec2f& min, const GfVec2f& max, unsigned flags)
  : _parent(parent)
  , _tab(NULL)
  , _left(NULL)
  , _right(NULL)
  , _min(min)
  , _max(max)
  , _flags(flags)
  , _perc(0.5)
  , _fixed(-1)
  , _buffered(2)
  , _current(NULL)
  , _currentIdx(-1)
  , _fixedSizeFn(NULL)
{
  if(_parent)_window = _parent->_window;
  if (_flags & View::TAB)CreateTab();
}

View::View(View* parent, int x, int y, int w, int h, unsigned flags)
  : _parent(parent)
  , _tab(NULL)
  , _left(NULL)
  , _right(NULL)
  , _min(GfVec2f(x, y))
  , _max(GfVec2f(x+w, y+h))
  , _flags(flags)
  , _perc(0.5)
  , _fixed(-1)
  , _buffered(2)
  , _current(NULL)
  , _currentIdx(-1)
  , _fixedSizeFn(NULL)
{
  if(_parent)_window = _parent->_window;
  if (_flags & View::TAB)CreateTab();
}

View::~View()
{
  if (_left) delete _left;
  if (_right) delete _right;

  for (auto& ui : _uis)
    delete ui;

  if (_tab) delete _tab;
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
  case UIType::GRAPHEDITOR:
    _current = new GraphEditorUI(this);
    break;
  case UIType::CURVEEDITOR:
    _current = new CurveEditorUI(this);
    break;
  case UIType::CONTENTBROWSER:
    _current = new ContentBrowserUI(this);
    break;
  case UIType::DEMO:
    _current = new DemoUI(this);
    break;
  case UIType::PROPERTYEDITOR:
    _current = new PropertyEditorUI(this);
    break;
  case UIType::ATTRIBUTEEDITOR:
    _current = new AttributeEditorUI(this);
    break;
  case UIType::DEBUG:
    _current = new DebugUI(this);
    break;
  case UIType::ICON:
    _current = new IconUI(this);
    break;
  case UIType::TOOL:
    _current = new ToolUI(this);
    break;
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

void
View::SetViewportMessage(const std::string &message)
{
  for(auto& ui: _uis)
    if(ui->GetType() == UIType::VIEWPORT)
      ((ViewportUI*)ui)->SetMessage(message);
}

bool
View::Contains(int x, int y)
{
  if(x>=_min[0] && x<=_max[0] && y>=_min[1] && y<=_max[1])return true;
  else return false;
}

bool
View::Intersect(const GfVec2f& min, const GfVec2f& size)
{
  GfRange2f viewRange(GetMin(), GetMax());
  GfRange2f boxRange(min, min + size);
  return !boxRange.IsOutside(viewRange);
}

bool
View::DrawTab()
{
  if (_tab) return (!_tab->Draw());
  return true;
}

void 
View::Draw(bool force)
{
  if (!GetFlag(LEAF)) {
    if (_left)_left->Draw(force);
    if (_right)_right->Draw(force);
  }
  if(_tab) DrawTab();

  if (_current && (GetFlag(INTERACTING) || GetFlag(DIRTY))) {
    bool isPlaying = Time::Get()->IsPlaying();
    bool isTimeVarying = GetFlag(TIMEVARYING) && isPlaying;
    bool isEdited = _current->Draw();

    bool doClean = !force && !IsActive() && !isEdited && !isTimeVarying;

    if ( doClean) 
      SetClean();
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

void View::Clear()
{
  if (_left) delete _left;
  if (_right) delete _right;
  _left = _right = NULL;

  for(auto& ui: _uis)
    delete ui;
  _uis.clear();
  _current = NULL;
  _currentIdx = -1;
  SetFlag(LEAF);
}

// mouse positon relative to the view
GfVec2f 
View::GetRelativeMousePosition(const int inX, const int inY)
{
  GfVec2f position = GetMin();
  int x = position[0];
  int y = position[1];
  return GfVec2f(inX - x, inY - y);
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
    
    if (_tab->Invade() || (relativeY > 0 && relativeY < GetTabHeight())) {
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
View::GetChildMinMax(bool leftOrRight, GfVec2f& cMin, GfVec2f& cMax)
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
View::GetSplitInfos(GfVec2f& sMin, GfVec2f& sMax,
  const int width, const int height)
{
  if(GetFlag(HORIZONTAL))
  { 
    sMin[0] = GetMin()[0] - SPLITTER_THICKNESS;
    sMax[0] = GetMax()[0] + SPLITTER_THICKNESS;
  
    int h = GetMin()[1] + (GetMax()[1] - GetMin()[1]) * GetPerc();
    sMin[1] = h - SPLITTER_THICKNESS;
    sMax[1] = h + SPLITTER_THICKNESS;
  }
  else
  {
    sMin[1] = GetMin()[1] - SPLITTER_THICKNESS;
    sMax[1] = GetMax()[1] + SPLITTER_THICKNESS;

    int w = GetMin()[0] + (GetMax()[0] - GetMin()[0]) * GetPerc();
    sMin[0] = w - SPLITTER_THICKNESS;
    sMax[0] = w + SPLITTER_THICKNESS;
  }

  sMin[0] = GfClamp(sMin[0], 0, width);
  sMin[1] = GfClamp(sMin[1], 0, height);
  sMax[0] = GfClamp(sMax[0], 0, width);
  sMax[1] = GfClamp(sMax[1], 0, height);

}

void
View::Split(double perc, bool horizontal, int fixed, int pixels)
{
  if(horizontal)SetFlag(HORIZONTAL);
  else ClearFlag(HORIZONTAL);

  if (fixed && pixels > 0) {
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
  _fixed = pixels;

  GfVec2f cMin, cMax;    
  GetChildMinMax(true, cMin, cMax);
  _left = new View(this, cMin, cMax);
  _left->_parent = this;

  GetChildMinMax(false, cMin, cMax);
  _right = new View(this, cMin, cMax);
  _right->_parent = this;

  ComputePixels();
  ClearFlag(LEAF);

  if (_tab && _left->_tab) {
    _left->TransferUIs(this);
  }
  RemoveTab();
  _window->Resize(_window->GetWidth(), _window->GetHeight());
}

void 
View::Resize(int x, int y, int w, int h)
{
  _min = GfVec2f(x , y);
  _max = GfVec2f(x + w, y + h);

  if(!GetFlag(LEAF))
  {
    if(_left)_left->ComputeFixedSize();
    if(_right)_right->ComputeFixedSize();
    
    if(GetFlag(HORIZONTAL))
    {
      if (GetFlag(LFIXED)) _perc = (double)_left->GetFixedSize() / (double)h;
      else if (GetFlag(RFIXED)) _perc = ((double)h - _right->GetFixedSize()) / (double)h;
      
      double ph = (double)h * _perc;
      if (_left)_left->Resize(x, y, w, ph);
      if (_right)_right->Resize(x, y + ph, w, h - ph);
    }
    else
    {
      if (GetFlag(LFIXED)) _perc = (float)_left->GetFixedSize() / (float)w;
      else if (GetFlag(RFIXED)) _perc = (float)(w - _right->GetFixedSize()) / (float)w;
      
      double pw = (double)w * _perc;
      if (_left)_left->Resize(x, y, pw, h);
      if (_right)_right->Resize(x + pw, y, w - pw, h);
    }
  }
  else
  {
    if(_current)_current->Resize();
  }
  SetFlag(DIRTY);
}

void 
View::GetPercFromMousePosition(int x, int y)
{
  if(GetFlag(HORIZONTAL))
  {
    if(GetHeight()<0.01)return;
    double perc = ((double)y - _min[1]) / ((double)_max[1] - _min[1]);
    _perc = perc < 0.05 ? 0.05 : perc > 0.95 ? 0.95 : perc;
  }
  else
  {
    if(GetWidth()<0.01)return;
    double perc = ((double)x - _min[0]) / ((double)_max[0] - _min[0]);
    _perc = perc < 0.05 ? 0.05 : perc > 0.95 ? 0.95 : perc;
  }
  ComputePixels();
 
}

void 
View::SetPerc(double perc)
{
  _perc=perc;
  ComputePixels();
};

void
View::ComputePixels()
{
  double side = GetFlag(HORIZONTAL) ? GetHeight() : GetWidth();

  if (GetFlag(LFIXED)) {
    _pixels[0] = _fixed;
    _pixels[1] = side - _fixed;
  }
  else if (GetFlag(RFIXED)) {
    _pixels[0] = side - _fixed;
    _pixels[1] = _fixed;
  }
  else {
    _pixels[0] = std::ceil(_perc * side);
    _pixels[1] = std::ceil((1 - _perc) * side);
  }
}

void 
View::RescalePixels(GfVec2d  ratio)
{
  if(!GetFlag(LEAF))
  {
    if(GetFlag(HORIZONTAL)){_pixels[0] *= ratio[1]; _pixels[1] *= ratio[1];}
    else {_pixels[0] *= ratio[0]; _pixels[1] *= ratio[0];}

    if(_left)_left->RescalePixels(ratio);
    if(_right)_right->RescalePixels(ratio);
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
      perc = (double)_left->_pixels[0] / height;
    }
    else { 
      double width = GetWidth() * _perc;
      perc = (double)_left->_pixels[0] / width;
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
      perc = 1.0-(double)_right->_pixels[1] / (double)height;
    }
    else { 
      double width = GetWidth() * (1.0 - _perc);
      perc = 1.0 - (double)_right->_pixels[1] / (double)width;
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
View::ComputeFixedSize()
{
  if(_fixedSizeFn)
    _fixed = (*_fixedSizeFn)(GetCurrentUI());
}

int
View::GetFixedSize() {
  ComputeFixedSize();
  return _fixed;
}

void
View::SetClean()
{
  if(!GetFlag(View::LEAF))return;
  if(!_current || WindowRegistry::IsPlaybackView(this))return;
  _buffered--;
  if (_buffered <= 0)
    ClearFlag(DIRTY);
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