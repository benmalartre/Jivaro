#ifndef AMN_UI_UI_H
#define AMN_UI_UI_H
#pragma once

#include <pxr/base/tf/notice.h>
#include <pxr/base/tf/weakBase.h>
#include <pxr/base/tf/instantiateType.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../common.h"
#include "../app/notice.h"

AMN_NAMESPACE_OPEN_SCOPE  

#define AMN_UI_HEADER_HEIGHT 32

class View;
class Window;
class Application;

enum UIType {
  VIEWPORT,
  EXPLORER,
  TIMELINE,
  PROPERTY,
  TOOLBAR,
  FILE_BROWSER,
  CURVE_EDITOR,
  GRAPH_EDITOR,
  COUNT
};

static const char* UITypeName[UIType::COUNT] = {
  "viewport",
  "explorer",
  "timeline",
  "property",
  "toolbar",
  "file browser",
  "curve editor",
  "graph editor"
};

class BaseUI : public pxr::TfWeakBase
{
public:
  BaseUI(View* parent, const std::string& name);
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
  virtual void GetRelativeMousePosition(const float inX, const float inY, 
    float& outX, float& outY);

  // get the (x,y) position in window space (left top corner)
  virtual ImVec2 GetPosition();

  // get the x position in window space (x-coordinate of left top corner)
  virtual int GetX();

  // get the y position in window space (y-coordinate of left top corner)
  virtual int GetY();

  // get the width of the parent view
  virtual int GetWidth();
  
  // get the height of the parent view
  virtual int GetHeight();

  Application* GetApplication();

  const std::string& GetName() const {return _name;};

  virtual void MouseButton(int button, int action, int mods){};
  virtual void MouseMove(int x, int y){}; 
  virtual void MouseWheel(int x, int y){};
  virtual void Keyboard(int key, int scancode, int action, int mods) {};

  bool DrawHead();
  virtual bool Draw()=0;
  virtual void Resize(){};

  // notices callbacks
  void OnNewSceneNotice(const Notice::NewScene& n);
  void OnAllNotices(const pxr::TfNotice& n);

protected:
  bool                    _initialized;
  View*                   _parent;
  std::string             _name;
};


AMN_NAMESPACE_CLOSE_SCOPE

#endif