#pragma once

#include "../default.h"
#include "../imgui/imgui_custom.h"
#include "../imgui/imgui_test.h"

AMN_NAMESPACE_OPEN_SCOPE  

class View;
class Window;
class BaseUI
{
public:
  BaseUI(View* parent, const std::string& name, bool docked=true);
  virtual ~BaseUI(){};

  // get parent window
  Window* GetWindow();

  // get parent window height
  int GetWindowHeight();

  // mouse position in the view space
  // (0, 0) left top corner
  // (width, height) right bottom corner
  void GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY);

  // get the x position in window space (x-coordinate of left top corner)
  int GetX();

  // get the y position in window space (y-coordinate of left top corner)
  int GetY();

  // get the width of the parent view
  int GetWidth();
  
  // get the height of the parent view
  int GetHeight();

  const std::string& GetName() const {return _name;};

  virtual void MouseButton(int action, int button, int mods){};
  virtual void MouseMove(int x, int y){}; 
  virtual void MouseWheel(int x, int y){};

  virtual void Draw()=0;
  virtual void Resize(){};
  

protected:
  View*             _parent;
  std::string       _name;
  bool              _docked;
  ImGuiWindowFlags  _flags;
  
};

AMN_NAMESPACE_CLOSE_SCOPE