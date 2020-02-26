#pragma once

#include "default.h"
#include "splitter.h"
#include "glutils.h"
#include "tools.h"

#include <pxr/pxr.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usd/prim.h>

#include <GLFW/glfw3.h>



namespace AMN {
  class Application;
  class View;
  class Splitter;


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

    // infos
    void GetContextVersionInfos();
    GLFWwindow* GetWindow(){return _window;};
    bool GetDebounce(){return _debounce;};
    void SetDebounce(bool debounce){_debounce=debounce;};
    void CollectLeaves(View* view);

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
    void RebuildSplittersMap();

    // splitters
    void SplitView(View* view, unsigned perc = 50, bool horizontal=true );
    View* GetMainView(){return _mainView;};
    View* GetActiveView(){return _activeView;};
    void MakeTextureFromPixels();

    // draw
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

    static Window* GetUserData(GLFWwindow* window);

  private:
    GLFWwindow*             _window;
    GLFWcursor*             _cursor;
    View*                   _mainView;
    View*                   _activeView;
    Splitter*               _splitter;
    std::vector<View*>      _leaves;

    // render settings
    //Camera            _camera;
    //Shader _shader;

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
    static Window* CreateFullScreenWindow();
    static Window* CreateStandardWindow(int width, int height);
  };

} // namespace AMN
