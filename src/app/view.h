#ifndef JVR_APPLICATION_VIEW_H
#define JVR_APPLICATION_VIEW_H
#pragma once

#include "../common.h"
#include "../ui/splitter.h"
#include "../ui/utils.h"
#include <pxr/usd/usd/prim.h>

JVR_NAMESPACE_OPEN_SCOPE

class Window;
class BaseUI;
class View
{
public:
  enum FLAGS
  {
    HORIZONTAL      = 1 << 1,
    OVER            = 1 << 2,
    LEAF            = 1 << 3,
    LFIXED          = 1 << 4,
    RFIXED          = 1 << 5,
    ACTIVE          = 1 << 6,
    DIRTY           = 1 << 7,
    INTERACTING     = 1 << 8,
    FORCEREDRAW     = 1 << 9
  };

  View(View* parent, const pxr::GfVec2f& min, const pxr::GfVec2f& max);
  View(View* parent, int x, int y, int w, int h);
  virtual ~View();
  void SetWindow(Window* Window);
  Window* GetWindow();
  const float GetX(){return _min[0];};
  const float GetY(){return _min[1];};
  const pxr::GfVec2f GetMin(){return _min;};
  const pxr::GfVec2f GetMax(){return _max;};
  float GetWidth(){return (_max[0] - _min[0]);};
  float GetHeight(){return (_max[1] - _min[1]);};
  pxr::GfVec2f GetSize(){return (_max - _min);};
  bool IsFixed(){return (_fixedPixels > 0);};

  inline const std::string& GetName(){return _name;};
  inline const char* GetText(){return _name.c_str();};
  inline double GetPerc(){return _perc;};
  void SetPerc(double perc);
  void GetPercFromMousePosition(int x, int y);
  void ComputeNumPixels(bool postFix=false);
  void RescaleNumPixels( pxr::GfVec2f ratio);
  void RescaleLeft();
  void RescaleRight();

  void GetChildMinMax(bool , pxr::GfVec2f& , pxr::GfVec2f&);
  void Split(double perc, bool horizontal, int fixed=false, int numPixels=-1);
  void GetSplitInfos(pxr::GfVec2f& sMin, pxr::GfVec2f& sMax, 
    const int width, const int height);

  inline View* GetLeft(){return _left;};
  inline View* GetRight(){return _right;};
  inline View* GetParent(){return _parent;};
  inline bool HasParent(){return _parent != NULL;};

  // content
  void SetContent(BaseUI* ui);
  BaseUI* GetContent(){return _content;};

  // cursor
  void GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY);
  bool Contains(int x, int y);
  
  // callbacks
  virtual void Draw(bool forceRedraw);
  virtual void Resize(int x, int y, int width, int height, bool rationalize=false);
  virtual void MouseMove(int x, int y);
  virtual void MouseButton(int button, int action, int mods);
  virtual void MouseWheel(int x, int y);
  virtual void Keyboard(int key, int scancode, int action, int mods);

  // flags
  inline bool GetFlag(short flag) const { return BITMASK_CHECK(_flags, flag); };
  inline void SetFlag(short flag) { BITMASK_SET(_flags, flag); };
  inline void ClearFlag(short flag) { BITMASK_CLEAR(_flags, flag); };

  void SetClean();
  void SetDirty();
  void SetInteracting(bool value);

private:
  pxr::GfVec2f      _min;
  pxr::GfVec2f      _max;
  unsigned          _flags;
  double            _perc;
  double            _lastPerc;
  unsigned          _numPixels[2];
  int               _fixedPixels;
  char              _buffered;
  BaseUI*           _content;
  Window*           _window;
  View*             _left;
  View*             _right;
  View*             _parent;
  std::string       _name;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_VIEW_H