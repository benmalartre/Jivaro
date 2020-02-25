#pragma once

#include "default.h"
#include "utils.h"
#include <pxr/usd/usd/prim.h>

namespace AMN {
  class Window;
  class UI;
  class View
  {
  enum FLAGS
  {
    OVER = 1,
    HORIZONTAL = 2,
    LEAF = 4
  };

  enum BORDER
  {
    TOP     = 1,
    RIGHT   = 2,
    BOTTOM  = 4,
    LEFT    = 8
  };

  public:
    View(View* parent, const pxr::GfVec2i& min, const pxr::GfVec2i& max);
    View(View* parent, int x, int y, int w, int h);
    const pxr::GfVec2i& GetMin(){return _min;};
    const pxr::GfVec2i& GetMax(){return _max;};
    const pxr::GfVec3f& GetColor(){return _color;};
    int GetPerc(){return _perc;};
    void SetPerc(float perc){_perc=perc;};
    void GetChildMinMax(bool , pxr::GfVec2i& , pxr::GfVec2i& );
    void Split();
    void Draw();
    int TouchBorder();
    void MouseEnter();
    void MouseLeave();

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

  private:
    pxr::GfVec2i      _min;
    pxr::GfVec2i      _max;
    pxr::GfVec3f      _color;
    int               _perc;
  protected:
    UI*               _content;
    View*             _left;
    View*             _right;
    View*             _parent;
    int               _flags;
  };

  class ViewMap
  {
  public: 
    ViewMap():_pixels(NULL){};
    ~ViewMap(){if(_pixels)delete [] _pixels;};

    void AddView(View* view);
    void BuildMap(int width, int height);
    
  private:
    Window* _window;
    const char* _pixels;
    std::map<char, View*> _map;
  };

}
