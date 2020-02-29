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

    // get parent window height
    int GetWindowHeight();

    // 
    //void SetWindowContext();
    void GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY);
    int GetX();
    int GetY();
    int GetWidth();
    int GetHeight();

    virtual void Event()=0;
    virtual void Draw()=0;

    

  protected:
    View*       _parent;
    std::string _name;
    
  };

} // namespace AMN
