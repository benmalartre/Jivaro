#pragma once

#include "imgui/imgui_custom.h"
#include "imgui/imgui_test.h"

namespace AMN {
  
  class View;

  class UI
  {
  public:
    UI(View* parent, const std::string& name);
    virtual ~UI(){};

    virtual void OnKeyboard()=0;
    virtual void OnMouseMove()=0;
    virtual void OnClick()=0;
    virtual void OnEnter()=0;
    virtual void OnLeave()=0;
    virtual void OnDraw()=0;

  protected:
    View*       _parent;
    std::string _name;
    
  };

} // namespace AMN
