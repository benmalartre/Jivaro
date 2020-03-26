#pragma once

#include "../common.h"
#include "../utils/utils.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

class Window;
class BaseUI;
class View
{
enum FLAGS
{
  OVER        = 1,
  HORIZONTAL  = 2,
  LEAF        = 4,
  FIXED       = 8
};

public:
  View(View* parent, const pxr::GfVec2f& min, const pxr::GfVec2f& max);
  View(View* parent, int x, int y, int w, int h);
  ~View();
  void SetWindow(Window* Window);
  Window* GetWindow();
  const float GetX(){return _min[0];};
  const float GetY(){return _min[1];};
  const pxr::GfVec2f& GetMin(){return _min;};
  const pxr::GfVec2f& GetMax(){return _max;};
  const pxr::GfVec3f& GetColor(){return _color;};
  float GetWidth(){return _max[0] - _min[0];};
  float GetHeight(){return _max[1] - _min[1];};
  pxr::GfVec2f GetSize(){return _max - _min;};

  inline const pxr::GfVec4f 
  GetColor4(){return pxr::GfVec4f(_color[0], _color[1], _color[2], 1.f);};

  inline const std::string& GetName(){return _name;};
  inline const char* GetText(){return _name.c_str();};
  inline double GetPerc(){return _perc;};
  void SetPerc(double perc);
  void GetPercFromMousePosition(int x, int y);
  void ComputeNumPixels(bool postFix=false);
  void RescaleNumPixels( pxr::GfVec2f ratio);
  void FixLeft();
  void FixRight();

  void GetChildMinMax(bool , pxr::GfVec2f& , pxr::GfVec2f& );
  void Split(double perc, bool horizontal, bool fixed);
  void GetSplitInfos(pxr::GfVec2f& sMin, pxr::GfVec2f& sMax, 
  const int width, const int height);

  inline View* GetLeft(){return _left;};
  inline View* GetRight(){return _right;};
  inline View* GetParent(){return _parent;};
  inline bool HasParent(){return _parent != NULL;};

  void GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY);

  void SetContent(BaseUI* ui);
  BaseUI* GetContent(){return _content;};

  // 
  bool Contains(int x, int y);
  
  void Draw();
  void Resize(int x, int y, int width, int height, bool rationalize=false);
  int TouchBorder();
  void MouseMove(int x, int y);
  void MouseButton(int action, int button, int mods);
  void MouseWheel(int x, int y);

  // FLAGS
  inline bool IsOver(){return BITMASK_CHECK(_flags, OVER);};
  inline void SetOver(){BITMASK_SET(_flags, OVER);};
  inline bool ClearOver(){return BITMASK_CLEAR(_flags, OVER);};

  inline bool IsHorizontal(){return BITMASK_CHECK(_flags, HORIZONTAL);};
  inline void SetHorizontal(){BITMASK_SET(_flags, HORIZONTAL);};
  inline bool ClearHorizontal(){return BITMASK_CLEAR(_flags, HORIZONTAL);};

  inline bool IsLeaf(){return BITMASK_CHECK(_flags, LEAF);};
  inline void SetLeaf(){BITMASK_SET(_flags, LEAF);};
  inline bool ClearLeaf(){return BITMASK_CLEAR(_flags, LEAF);};

  inline bool IsFixed(){return BITMASK_CHECK(_flags, FIXED);};
  inline void SetFixed(){BITMASK_SET(_flags, FIXED);};
  inline bool ClearFixed(){return BITMASK_CLEAR(_flags, FIXED);};

private:
  pxr::GfVec2f      _min;
  pxr::GfVec2f      _max;
  pxr::GfVec3f      _color;
  double            _perc;
  double            _lastPerc;
  unsigned          _npixels[2];
  BaseUI*           _content;
  View*             _left;
  View*             _right;
  View*             _parent;
  unsigned          _flags;
  std::string       _name;
  Window*           _window;
};

AMN_NAMESPACE_CLOSE_SCOPE