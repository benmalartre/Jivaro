#ifndef JVR_APP_WINDOW_H
#define JVR_APP_WINDOW_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/fonts.h"
#include "../ui/utils.h"
#include "../app/tools.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/glContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

JVR_NAMESPACE_OPEN_SCOPE

extern bool LEGACY_OPENGL;

class UsdEmbreeContext;
class Application;
class View;
class SplitterUI;
class BaseUI;
class PopupUI;

// keyboard callback
//----------------------------------------------------------------------------
JVR_EXPORT void
KeyboardCallback(GLFWwindow* window, int key, int code, int action, int mods);

// button callback
//----------------------------------------------------------------------------
JVR_EXPORT void 
ClickCallback(GLFWwindow* window, int button, int action, int mods);

// mouse move callback
//----------------------------------------------------------------------------
JVR_EXPORT void 
MouseMoveCallback(GLFWwindow* window, double x, double y);

// display callback
//----------------------------------------------------------------------------
JVR_EXPORT void 
DisplayCallback(GLFWwindow* window);

// scroll callback
//----------------------------------------------------------------------------
JVR_EXPORT void 
ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

// char callback
//----------------------------------------------------------------------------
JVR_EXPORT void 
CharCallback(GLFWwindow* window, unsigned c);

// resize callback
//----------------------------------------------------------------------------
JVR_EXPORT void 
WindowSizeCallback(GLFWwindow* window, int width, int height);

// focus callback
//----------------------------------------------------------------------------
JVR_EXPORT void
FocusCallback(GLFWwindow* window, int focused);

enum WindowLayout {
  WINDOW_LAYOUT_BASE,
  WINDOW_LAYOUT_RAW,
  WINDOW_LAYOUT_STANDARD,
  WINDOW_LAYOUT_RANDOM
};

class Window
{
public:
  // constructor
  Window(const std::string& name, const GfVec4i& dimension,
    bool fullscreen=false, Window* parent=NULL);

  //destructor
  ~Window();

  // initialize
  void Init();

  // ui
  ImGuiContext* GetContext() { return _context; };
  std::string ComputeUniqueUIName(short type);

  // tool
  Tool* GetTool() { return &_tool; };

  // infos
  void GetContextVersionInfos();
  GLFWwindow* GetGlfwWindow(){return _window;};
  bool GetDebounce(){return _debounce;};
  void SetDebounce(bool debounce){_debounce=debounce;};
  const std::string& GetName() { return _name; };

  // imgui context
  void SetupImgui();
  void ClearImgui();

  // fullscreen
  bool IsFullScreen(){return _fullscreen;};
  void SetFullScreen(bool fullscreen){_fullscreen = fullscreen;};

  // size
  int GetWidth(){return _width;};
  int GetHeight(){return _height;};
  GfVec2i GetResolution() { return GfVec2i(_width, _height); };
  void SetWidth(int width){_width = width;};
  void SetHeight(int height){_height = height;};
  void Resize(unsigned width, unsigned height);

  int GetMenuBarHeight();

  // views
  SplitterUI* GetSplitter(){return _splitter;};
  View* SplitView(View* view, double perc = 0.5, bool horizontal=true, 
    int fixed=0, int numPixels=-1);
  void RemoveView(View* view);
  View* GetMainView(){return _mainView;};
  void SetActiveView(View* view);
  void SetHoveredView(View* view);
  View* GetActiveView(){return _activeView;};
  View* GetHoveredView() { return _hoveredView; };
  View* GetViewUnderMouse(int x, int y);
  void CollectLeaves();
  const std::vector<View*>& GetLeaves();
  const std::vector<View*>& GetViews();
  void DirtyViewsUnderBox(const GfVec2f& min, const GfVec2f& size);
  void DiscardMouseEventsUnderBox(const GfVec2f& min, const GfVec2f& size);
  void DiscardKeyboardEvents();
  void InvalidateViews();
  void ClearViews();
  void SetLayout();
  void SetDesiredLayout(size_t layout);
  size_t GetLayout();
  void DirtyAllViews();
  
  // draw
  void SetGLContext();
  void Draw(bool force);
  void DrawPopup(PopupUI* popup);
  bool PickSplitter(double mX, double mY);
  void SetIdle(bool value){_idle=value;};
  bool IsIdle(){return _idle;};
  void CaptureFramebuffer();
  void BeginRepeatKey();
  void EndRepeatKey();
  bool ShouldRepeatKey();
  void SetViewportMessage(const std::string &message);

  // fonts
  inline ImFont* GetFont(size_t size, size_t index);

  // tool
  inline void SetActiveTool(int tool) {
    if(tool != _activeTool) {
      _lastActiveTool = _activeTool;
      _activeTool = tool;
    }
  };
  inline int GetActiveTool(){return _activeTool;};
  inline void RestoreLastActiveTool(){_activeTool=_lastActiveTool;};
  void UpdateActiveTool(int mouseX, int mouseY);

  // splitter
  void BeginDragSplitter(){_dragSplitter=true;};
  void EndDragSplitter() { _dragSplitter = false;};
  bool IsDraggingSplitter(){return _dragSplitter;};
  void DragSplitter(int x, int y);

  // update
  bool Update();
  static Window* GetUserData(GLFWwindow* window);

private:
  bool                  _idle;
  std::string           _name;
  GLFWwindow*           _window;
  bool                  _shared;
  View*                 _mainView;
  View*                 _activeView;
  View*                 _hoveredView;
  View*                 _activeLeaf;
  SplitterUI*           _splitter;
  bool                  _dragSplitter;
  std::vector<View*>    _views;
  std::vector<View*>    _leaves;
  ImGuiContext*         _context;
  Tool                  _tool;

  // view datas
  bool                  _fullscreen;
  int                   _mouseMode;
  int                   _activeTool;
  int                   _lastActiveTool;
  int                   _width;
  int                   _height;
  unsigned*             _pixels;
  bool                  _valid;
  int                   _layout;
  bool                  _needUpdateLayout;

  // version number
  int                   _iOpenGLMajor;
  int                   _iOpenGLMinor;
  int                   _iOpenGLRevision;

  // opengl
  GLuint                _vao;

  // imgui
  ImGuiIO*              _io;
  bool                  _debounce;
  uint64_t              _lastRepeatT;

  // fonts
  float                 _fontSize;

  // ui
  GLuint                _fbo;
  GLuint                _tex;
  UITypeCounter         _uic;

public:
  // static constructor
  //----------------------------------------------------------------------------
  static Window* CreateFullScreenWindow(const std::string& name);
  static Window* CreateStandardWindow(const std::string& name, const GfVec4i& dimension);
  static Window* CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent);
};

// fonts
ImFont* Window::GetFont(size_t size, size_t index) 
{ 
  if(size == FONT_SMALL)
    return FONTS_SMALL[index];
  else if(size == FONT_MEDIUM)
    return FONTS_MEDIUM[index];
  else
    return FONTS_LARGE[index];
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APP_WINDOW_H