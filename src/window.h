#pragma once

#include "default.h"
#include <GLFW/glfw3.h>
#include "view.h"
#include "pxr/pxr.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/usd/usd/prim.h>


namespace AMN {
  class Application;
  class Window;
  class View;


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
  MotionCallback(GLFWwindow* window, double x, double y);

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
  ReshapeCallback(GLFWwindow* window, int width, int height);
  
  class Splitter : pxr::UsdPrim
  {
    pxr::GfVec2i  _min;
    pxr::GfVec2i  _max;
    int         _perc;
    Window*     _left;
    Window*     _right;
  };

  class Window : pxr::UsdPrim
  {
  public:
    Window(int width, int height);
    Window(bool fullscreen);
    ~Window();

    // infos
    void GetContextVersionInfos();
    GLFWwindow* GetWindow(){return _window;};
    bool GetDebounce(){return _debounce;};
    void SetDebounce(bool debounce){_debounce=debounce;};

    // imgui context
    void SetupImgui();
    void ClearImgui();
    void TestImgui(int index);
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

    // loop in thread
    void MainLoop();

    static Window* GetUserData(GLFWwindow* window);

  private:
    std::vector<View> _views;
    GLFWwindow*       _window;

    // render settings
    //Camera            _camera;
    //Shader _shader;

    bool              _fullscreen;
    int               _width;
    int               _height;
    unsigned*         _pixels;

    // version number
    int               _iOpenGLMajor;
    int               _iOpenGLMinor;
    int               _iOpenGLRevision;

    // imgui
    ImGuiIO*          _io;
    int               _guiId;
    bool              _debounce;

  public:
    // static constructor
    //----------------------------------------------------------------------------
    static Window* CreateFullScreenWindow();
    static Window* CreateStandardWindow(int width, int height);

  };

  static void WindowExecution(Window* window)
  {
      window->MainLoop();
  };
  

} // namespace AMN
