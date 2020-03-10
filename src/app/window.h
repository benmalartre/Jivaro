#pragma once

#include "../default.h"
#include "../utils/utils.h"
#include "splitter.h"
#include "tools.h"
#include "ui.h"
#include "../imgui/imgui_nodes.h"
#include "../ui/font.h"

#include <pxr/pxr.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usd/prim.h>

#include <GLFW/glfw3.h>

AMN_NAMESPACE_OPEN_SCOPE

class AmnUsdEmbreeContext;
class AmnApplication;
class AmnView;
class AmnSplitter;
class AmnUI;

extern AmnUsdEmbreeContext* EMBREE_CTXT;

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

class AmnWindow
{
public:
  // constructor
  AmnWindow(int width, int height);
  AmnWindow(bool fullscreen);

  //destructor
  ~AmnWindow();

  // initialize
  void Init();
  void DummyFill();

  // infos
  void GetContextVersionInfos();
  GLFWwindow* GetGlfwWindow(){return _window;};
  bool GetDebounce(){return _debounce;};
  void SetDebounce(bool debounce){_debounce=debounce;};
  void CollectLeaves(AmnView* view = NULL);

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

  // splitters
  void SplitView(AmnView* view, unsigned perc = 50, bool horizontal=true );
  AmnView* GetMainView(){return _mainView;};
  AmnView* GetActiveView(){return _activeView;};
  void MakeTextureFromPixels();

  // draw
  void SetContext();
  void Draw();
  void DrawPickImage();
  bool PickSplitter(double mX, double mY);
  void ScreenSpaceQuad();

  // tool
  inline void SetActiveTool(int tool){_activeTool = tool;};
  inline int GetActiveTool(){return _activeTool;};
  bool UpdateActiveTool(int mouseX, int mouseY);
  // loop in thread
  void MainLoop();
  static AmnWindow* GetUserData(GLFWwindow* window);

private:
  // objects
  GLFWwindow*                _window;
  GLFWcursor*                _cursor;
  AmnView*                   _mainView;
  AmnView*                   _activeView;
  AmnView*                   _activeLeaf;
  AmnSplitter*               _splitter;
  std::vector<AmnView*>      _leaves;

  // view datas
  bool              _fullscreen;
  int               _mouseMode;
  int               _activeTool;
  int               _clickX;
  int               _clickY;
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

public:
  // static constructor
  //----------------------------------------------------------------------------
  static AmnWindow* CreateFullScreenWindow();
  static AmnWindow* CreateStandardWindow(int width, int height);
};

AMN_NAMESPACE_CLOSE_SCOPE