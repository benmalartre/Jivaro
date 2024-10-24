#ifndef JVR_APPLICATION_VIEW_H
#define JVR_APPLICATION_VIEW_H
#include <pxr/usd/usd/prim.h>

#include <vector>
#include "../common.h"
#include "../ui/utils.h"
#include "../ui/tab.h"


JVR_NAMESPACE_OPEN_SCOPE

class Window;
class BaseUI;
class ViewTabUI;
class View
{
public:
  enum Flags
  {
    HORIZONTAL          = 1 << 1,
    OVER                = 1 << 2,
    LEAF                = 1 << 3,
    LFIXED              = 1 << 4,
    RFIXED              = 1 << 5,
    ACTIVE              = 1 << 6,
    DIRTY               = 1 << 7,
    INTERACTING         = 1 << 8,
    FORCEREDRAW         = 1 << 9,
    TIMEVARYING         = 1 << 10,
    DISCARDMOUSEBUTTON  = 1 << 11,
    DISCARDMOUSEMOVE    = 1 << 12,
    TAB                 = 1 << 13,
    FOCUS               = 1 << 14
  };

  View(View* parent, const GfVec2f& min, const GfVec2f& max, 
    unsigned flags=HORIZONTAL|DIRTY|LEAF|TAB);
  View(View* parent, int x, int y, int w, int h, 
    unsigned flags= HORIZONTAL|DIRTY|LEAF|TAB);
  virtual ~View();
  void SetWindow(Window* Window);
  Window* GetWindow();
  View* GetSibling();
  const float GetX(){return _min[0];};
  const float GetY(){return _min[1];};
  const GfVec2f GetMin(){return _min;};
  const GfVec2f GetMax(){return _max;};
  float GetWidth(){return (_max[0] - _min[0]);};
  float GetHeight(){return (_max[1] - _min[1]);};
  const GfVec2f GetSize(){return (_max - _min);};
  bool IsFixed(){return (_fixedPixels > 0);};
  bool IsActive();
  bool IsHovered();

  inline double GetPerc(){return _perc;};
  void SetPerc(double perc);
  void GetPercFromMousePosition(int x, int y);
  void ComputeNumPixels(bool postFix=false);
  void RescaleNumPixels( GfVec2f ratio);
  void RescaleLeft();
  void RescaleRight();

  void GetChildMinMax(bool , GfVec2f& , GfVec2f&);
  void Split(double perc, bool horizontal, int fixed=false, int numPixels=-1);
  void GetSplitInfos(GfVec2f& sMin, GfVec2f& sMax, 
    const int width, const int height);

  void SetLeft(View* view){_left = view; if(view)view->_parent=this;};
  void SetRight(View* view){_right = view; if(view)view->_parent=this;};
  void SetParent(View* view){_parent = view;};
  inline View* GetLeft(){return _left;};
  inline View* GetRight(){return _right;};
  inline View* GetParent(){return _parent;};
  inline bool HasParent(){return _parent != NULL;};
  void Clear();

  // tab
  ViewTabUI* GetTab() { return _tab; };
  ViewTabUI* CreateTab();
  void RemoveTab();
  float GetTabHeight();

  // content
  void CreateUI(UIType type);
  void AddUI(BaseUI* ui);
  void RemoveUI(int index);
  void RemoveUI(BaseUI* ui);
  void SetCurrentUI(int index);
  BaseUI* GetCurrentUI();
  const std::vector<BaseUI*>& GetUIs() const;
  std::vector<BaseUI*>& GetUIs();
  void TransferUIs(View* source);
  bool HasUIs() { return _uis.size() > 0; };
  void SetViewportMessage(const std::string &message);

  // current
  BaseUI* GetCurrent() { return _current; };
  void SetCurrent(BaseUI* ui) { _current = ui; };

  // cursor
  GfVec2f GetRelativeMousePosition(const int inX, const int inY);
  bool Contains(int x, int y);
  bool Intersect(const GfVec2f& min, const GfVec2f& size);
  
  // callbacks
  bool DrawTab();
  virtual void Draw(bool forceRedraw);
  virtual void Resize(int x, int y, int width, int height, bool rationalize=false);
  virtual void MouseMove(int x, int y);
  virtual void MouseButton(int button, int action, int mods);
  virtual void MouseWheel(int x, int y);
  virtual void Keyboard(int key, int scancode, int action, int mods);
  virtual void Input(int key);
  virtual void Focus(int state);

  // flags
  inline bool GetFlag(short flag) const { return BITMASK_CHECK(_flags, flag); };
  inline void SetFlag(short flag) { BITMASK_SET(_flags, flag); };
  inline void ClearFlag(short flag) { BITMASK_CLEAR(_flags, flag); };
  unsigned GetFlags(){ return _flags;};
  void SetFlags(unsigned flags){_flags = flags;};

  void SetClean();
  void SetDirty();
  void SetTabed(bool tabed);
  void SetInteracting(bool value);
  bool IsInteracting();

private:
  GfVec2f          _min;
  GfVec2f          _max;
  unsigned              _flags;
  double                _perc;
  double                _lastPerc;
  unsigned              _numPixels[2];
  int                   _fixedPixels;
  int                   _buffered;
  ViewTabUI*            _tab;
  Window*               _window;
  View*                 _left;
  View*                 _right;
  View*                 _parent;
  BaseUI*               _current;
  std::vector<BaseUI*>  _uis;
  size_t                _currentIdx;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_VIEW_H