#ifndef JVR_APP_WINDOW_H
#define JVR_APP_WINDOW_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/fonts.h"
#include "../ui/utils.h"
#include "../ui/splitter.h"
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
class Splitter;
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
ResizeCallback(GLFWwindow* window, int width, int height);

// focus callback
//----------------------------------------------------------------------------
JVR_EXPORT void
FocusCallback(GLFWwindow* window, int focused);


class Window
{
public:
  // constructor
  Window(int width, int height, const std::string& name);
  Window(bool fullscreen, const std::string& name);
  Window(int x, int y, int width, int height,
    GLFWwindow* parent, const std::string& name, bool decorated=true);

  //destructor
  ~Window();

  // initialize
  void Init();

  // ui
  ImGuiContext* GetContext() { return _context; };

  // tool
  Tool* GetTool() { return &_tool; };

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
  //void AddChild(Window* child);
  //void RemoveChild(Window* child);

  // size
  int GetWidth(){return _width;};
  int GetHeight(){return _height;};
  pxr::GfVec2i GetResolution() { return pxr::GfVec2i(_width, _height); };
  void SetWidth(int width){_width = width;};
  void SetHeight(int height){_height = height;};
  void Resize(unsigned width, unsigned height);

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
  void DirtyViewsUnderBox(const pxr::GfVec2i& min, const pxr::GfVec2i& size);
  void DiscardMouseEventsUnderBox(const pxr::GfVec2i& min, const pxr::GfVec2i& size);
  
  // draw
  void SetGLContext();
  void Draw();
  void DrawPopup(PopupUI* popup);
  bool PickSplitter(double mX, double mY);
  void ForceRedraw();
  void SetIdle(bool value){_idle=value;};
  bool IsIdle(){return _idle;};
  void CaptureFramebuffer();

  // fonts
  inline ImFont* GetFont(size_t index){return FONTS[index];};

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
  int                   _forceRedraw;

  // version number
  int                   _iOpenGLMajor;
  int                   _iOpenGLMinor;
  int                   _iOpenGLRevision;

  // opengl
  GLuint                _vao;

  // imgui
  ImGuiIO*              _io;
  int                   _guiId;
  bool                  _debounce;

  // fonts
  float                 _fontSize;

  // ui
  float                 _dpiX;
  float                 _dpiY;
  GLuint                _fbo;
  GLuint                _tex;

public:
  // static constructor
  //----------------------------------------------------------------------------
  static Window* CreateFullScreenWindow();
  static Window* CreateStandardWindow(int width, int height);
  static Window* CreateChildWindow(int x, int y, int width, int height, GLFWwindow* parent,
    const std::string& name="Child", bool decorated=true);
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APP_WINDOW_H