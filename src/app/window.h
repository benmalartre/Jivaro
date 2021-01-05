#ifndef AMN_APP_WINDOW_H
#define AMN_APP_WINDOW_H
#pragma once

#include "../common.h"
#include "../utils/utils.h"
#include "../ui/ui.h"
#include "../ui/splitter.h"
#include "tools.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/glContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

AMN_NAMESPACE_OPEN_SCOPE

extern ImFontAtlas* AMN_SHARED_ATLAS;
extern ImFont* AMN_BOLD_FONTS[3];
extern ImFont* AMN_MEDIUM_FONTS[3];
extern ImFont* AMN_REGULAR_FONTS[3];

class UsdEmbreeContext;
class Application;
class View;
class Splitter;
class BaseUI;

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

// resize callback
//----------------------------------------------------------------------------
AMN_EXPORT void 
ResizeCallback(GLFWwindow* window, int width, int height);

class Window
{
public:
  // constructor
  Window(int width, int height, const std::string& name);
  Window(bool fullscreen, const std::string& name);
  Window(int width, int height, GLFWwindow* parent, const std::string& name);

  //destructor
  ~Window();

  // initialize
  void Init(Application* app);

  // application
  Application* GetApplication(){return _app;};

  // ui
  ImGuiContext* GetContext() { return _context; };

  // infos
  void GetContextVersionInfos();
  GLFWwindow* GetGlfwWindow(){return _window;};
  bool GetDebounce(){return _debounce;};
  void SetDebounce(bool debounce){_debounce=debounce;};
  void CollectLeaves(View* view = NULL);
  const std::string& GetName() { return _name; };

  // imgui context
  void SetupImgui();
  void ClearImgui();
  int GetGuiId(){return _guiId;};

  // fullscreen
  bool IsFullScreen(){return _fullscreen;};
  void SetFullScreen(bool fullscreen){_fullscreen = fullscreen;};

  // children
  void AddChild(Window* child);
  void RemoveChild(Window* child);

  // size
  int GetWidth(){return _width;};
  int GetHeight(){return _height;};
  void SetWidth(int width){_width = width;};
  void SetHeight(int height){_height = height;};
  void Resize(unsigned width, unsigned height);
  void BuildSplittersMap();

  // views
  Splitter* GetSplitter(){return _splitter;};
  View* SplitView(View* view, double perc = 0.5, bool horizontal=true, 
    int fixed=0, int numPixels=-1);
  View* GetMainView(){return _mainView;};
  void SetActiveView(View* view);
  View* GetActiveView(){return _activeView;};
  View* GetViewUnderMouse(int x, int y);
  
  // draw
  void SetGLContext();
  void Draw();
  bool PickSplitter(double mX, double mY);
  void ForceRedraw() { _forceRedraw = 3; };
  void SetIdle(bool value){_idle=value;};
  bool IsIdle(){return _idle;};

  // fonts
  inline ImFont* GetBoldFont(size_t index){return AMN_BOLD_FONTS[index];};
  inline ImFont* GetMediumFont(size_t index){return AMN_MEDIUM_FONTS[index];};
  inline ImFont* GetRegularFont(size_t index){return AMN_REGULAR_FONTS[index];};

  // tool
  inline void SetActiveTool(int tool) {
    if(tool != _activeTool) {
      _lastActiveTool = _activeTool;
      _activeTool = tool;
    }
  };
  inline int GetActiveTool(){return _activeTool;};
  inline void RestoreLastActiveTool(){_activeTool=_lastActiveTool;};
  bool UpdateActiveTool(int mouseX, int mouseY);

  // splitter
  void BeginDragSplitter(){_dragSplitter=true;};
  void EndDragSplitter(){_dragSplitter=false;};
  bool IsDraggingSplitter(){return _dragSplitter;};
  void DragSplitter(int x, int y);

  // loop
  void MainLoop();
  static Window* GetUserData(GLFWwindow* window);

private:
  bool                      _idle;
  std::string               _name;
  Application*              _app;
  GLFWwindow*               _window;
  bool                      _shared;
  std::vector<Window*>      _childrens;
  View*                     _mainView;
  View*                     _activeView;
  View*                     _activeLeaf;
  Splitter*                 _splitter;
  bool                      _dragSplitter;
  std::vector<View*>        _leaves;
  ImGuiContext*             _context;

  // view datas
  bool              _fullscreen;
  int               _mouseMode;
  int               _activeTool;
  int               _lastActiveTool;
  int               _width;
  int               _height;
  unsigned*         _pixels;
  bool              _valid;
  int               _forceRedraw;

  // version number
  int               _iOpenGLMajor;
  int               _iOpenGLMinor;
  int               _iOpenGLRevision;

  // opengl
  int               _pickImage;
  int               _debugImage;

  // imgui
  ImGuiIO*          _io;
  int               _guiId;
  bool              _debounce;

  // fonts
  float             _fontSize;

  // ui
  float             _dpiX;
  float             _dpiY;

public:
  // static constructor
  //----------------------------------------------------------------------------
  static Window* CreateFullScreenWindow();
  static Window* CreateStandardWindow(int width, int height);
  static Window* CreateChildWindow(int width, int height, GLFWwindow* parent,
    const std::string& name="Child");
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_APP_WINDOW_H