#pragma once

#include "../default.h"
#include "../imgui/imgui_custom.h"
#include "../imgui/imgui_test.h"

AMN_NAMESPACE_OPEN_SCOPE  

class AmnView;

class AmnUI
{
public:
  AmnUI(AmnView* parent, const std::string& name);
  virtual ~AmnUI(){};

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
  AmnView*       _parent;
  std::string _name;
  
};

AMN_NAMESPACE_CLOSE_SCOPE