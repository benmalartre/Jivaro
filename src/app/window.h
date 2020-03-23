#pragma once

#include "../default.h"
#include "../utils/utils.h"
#include "splitter.h"
#include "tools.h"
#include "ui.h"
#include "../imgui/imgui_nodes.h"

#include <pxr/pxr.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usd/prim.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

AMN_NAMESPACE_OPEN_SCOPE

class UsdEmbreeContext;
class Application;
class View;
class Splitter;
class BaseUI;

extern UsdEmbreeContext* EMBREE_CTXT;

// keyboard callback
//----------------------------------------------------------------------------
AMN_EXPORT void
KeyboardCallback(GLFWwindow* window, int key, int code, int action, int mods);

// button callback
//----------------------------------------------------------------------------
AMN_EXPORT void 
ClickCallback(GLFWwindow* window, int button, int action, int mods);

// mouse move callback
//----------------------------------------------------------------------------
AMN_EXPORT void 
MouseMoveCallback(GLFWwindow* window, double x, double y);

// display callback
//----------------------------------------------------------------------------
AMN_EXPORT void 
DisplayCallback(GLFWwindow* window);

// scroll callback
//----------------------------------------------------------------------------
AMN_EXPORT void 
ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

// char callback
//----------------------------------------------------------------------------
AMN_EXPORT void 
CharCallback(GLFWwindow* window, unsigned c);

// reshape callback
//----------------------------------------------------------------------------
AMN_EXPORT void 
ResizeCallback(GLFWwindow* window, int width, int height);

class Window
{
public:
  // constructor
  Window(int width, int height);
  Window(bool fullscreen);

  //destructor
  ~Window();

  // initialize
  void Init();
  void DummyFill();

  // infos
  void GetContextVersionInfos();
  GLFWwindow* GetGlfwWindow(){return _window;};
  bool GetDebounce(){return _debounce;};
  void SetDebounce(bool debounce){_debounce=debounce;};
  void CollectLeaves(View* view = NULL);

  // imgui context
  void SetupImgui();
  void ClearImgui();
  int GetGuiId(){return _guiId;};

  // fullscreen
  bool IsFullScreen(){return _fullscreen;};
  void SetFullScreen(bool fullscreen){_fullscreen = fullscreen;};

  // size
  int GetWidth(){return _width;};
  int GetHeight(){return _height;};
  void SetWidth(int width){_width = width;};
  void SetHeight(int height){_height = height;};
  void Resize(unsigned width, unsigned height);
  void BuildSplittersMap();

  // views
  Splitter* GetSplitter(){return _splitter;};
  View* SplitView(View* view, double perc = 0.5, bool horizontal=true );
  View* GetMainView(){return _mainView;};
  void SetActiveView(View* view);
  View* GetActiveView(){return _activeView;};
  View* GetViewUnderMouse(int x, int y);
  
  // draw
  void GetContentScale();
  void SetContext();
  void Draw();
  bool PickSplitter(double mX, double mY);

  // fonts
  inline ImFont* GetBoldFont(){return _boldFont;};
  inline ImFont* GetMediumFont(){return _mediumFont;};
  inline ImFont* GetRegularFont(){return _regularFont;};

  // tool
  inline void SetActiveTool(int tool){_activeTool = tool;};
  inline int GetActiveTool(){return _activeTool;};
  bool UpdateActiveTool(int mouseX, int mouseY);

  // loop in thread
  void MainLoop();
  static Window* GetUserData(GLFWwindow* window);

private:
  // objects
  GLFWwindow*             _window;
  View*                   _mainView;
  View*                   _activeView;
  View*                   _activeLeaf;
  Splitter*               _splitter;
  std::vector<View*>      _leaves;

  // view datas
  bool              _fullscreen;
  int               _mouseMode;
  int               _activeTool;
  int               _width;
  int               _height;
  unsigned*         _pixels;

  // version number
  int               _iOpenGLMajor;
  int               _iOpenGLMinor;
  int               _iOpenGLRevision;

  // opengl
  GLuint            _pickImage;
  GLuint            _debugImage;

  // imgui
  ImGuiIO*          _io;
  int               _guiId;
  bool              _debounce;

  // fonts
  ImFont*           _boldFont;
  ImFont*           _mediumFont;
  ImFont*           _regularFont;
  float             _fontSize;

  // ui
  float                   _dpiX;
  float                   _dpiY;

public:
  // static constructor
  //----------------------------------------------------------------------------
  static Window* CreateFullScreenWindow();
  static Window* CreateStandardWindow(int width, int height);
};

AMN_NAMESPACE_CLOSE_SCOPE