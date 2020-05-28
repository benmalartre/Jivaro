#pragma once

#include "../common.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

AMN_NAMESPACE_OPEN_SCOPE  

class View;
class Window;
class Application;
class BaseUI
{
public:
  BaseUI(View* parent, const std::string& name, bool docked=true);
  virtual ~BaseUI(){};

  // get parent window
  Window* GetWindow();

  // get the parent view
  View* GetView() { return _parent; };

  // get parent window height
  int GetWindowHeight();

  // mouse position in the view space
  // (0, 0) left top corner
  // (width, height) right bottom corner
  void GetRelativeMousePosition(const float inX, const float inY, float& outX, float& outY);

  // get the (x,y) position in window space (left top corner)
  ImVec2 GetPosition();

  // get the x position in window space (x-coordinate of left top corner)
  int GetX();

  // get the y position in window space (y-coordinate of left top corner)
  int GetY();

  // get the width of the parent view
  int GetWidth();
  
  // get the height of the parent view
  int GetHeight();

  Application* GetApplication();

  const std::string& GetName() const {return _name;};

  virtual void MouseButton(int button, int action, int mods){};
  virtual void MouseMove(int x, int y){}; 
  virtual void MouseWheel(int x, int y){};

  virtual bool Draw()=0;
  virtual void Resize(){};

protected:
  View*             _parent;
  std::string       _name;
  bool              _docked;
  ImGuiWindowFlags  _flags;
  
};


AMN_NAMESPACE_CLOSE_SCOPE