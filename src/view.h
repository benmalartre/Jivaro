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


  public:
    View(View* parent, const pxr::GfVec2i& min, const pxr::GfVec2i& max);
    View(View* parent, int x, int y, int w, int h);
    const pxr::GfVec2i& GetMin(){return _min;};
    const pxr::GfVec2i& GetMax(){return _max;};
    const pxr::GfVec3f& GetColor(){return _color;};
    const pxr::GfVec4f GetColor4()
    {
      return pxr::GfVec4f(_color[0], _color[1], _color[2], 1.f);
    };
    const std::string& GetName(){return _name;};
    const char* GetText(){return _name.c_str();};
    int GetPerc(){return _perc;};
    void SetPerc(float perc){_perc=perc;};
    void GetChildMinMax(bool , pxr::GfVec2i& , pxr::GfVec2i& );

    inline View* GetLeft(){return _left;};
    inline View* GetRight(){return _right;};
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
    std::string       _name;
  };

}
