#include "window.h"
#include "view.h"
#include "splitter.h"
#include "../utils/glutils.h"
#include "../utils/files.h"
#include "../widgets/dummy.h"
#include "../widgets/viewport.h"
#include "../imgui/imgui_custom.h"
#include "../imgui/imgui_test.h"
#include <pxr/base/arch/systemInfo.h>

JVR_NAMESPACE_OPEN_SCOPE

// fullscreen window constructor
//----------------------------------------------------------------------------
Window::Window(bool fullscreen) :
_pixels(NULL), _debounce(0),_mainView(NULL), _activeView(NULL), 
_pickImage(0),_splitter(NULL),_fontSize(16.f)
{
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  glfwWindowHint(GLFW_RED_BITS,mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS,mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS,mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE,mode->refreshRate);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);

  _window = glfwCreateWindow(mode->width, mode->height, "AMINA.0.0",  monitor, NULL);
  _width = mode->width;
  _height = mode->height;
  Init();
}

// width/height window constructor
//----------------------------------------------------------------------------
Window::Window(int width, int height):
_pixels(NULL), _debounce(0),_mainView(NULL), _activeView(NULL), 
_pickImage(0), _splitter(NULL),_fontSize(16.f)
{
  _width = width;
  _height = height;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  
  _window = glfwCreateWindow(_width,_height,"AMINA.0.0",NULL,NULL);
  Init();

}

// initialize
//----------------------------------------------------------------------------
void 
Window::Init()
{
  if(_window)
  {
    // create main splittable view
    _mainView = new View(NULL, pxr::GfVec2f(0,0), pxr::GfVec2f(_width, _height));
    _mainView->SetWindow(this);
    _splitter = new Splitter();
    
    // window datas
    GetContextVersionInfos();
    glfwSetWindowUserPointer(_window, this);
    
    // set current opengl context
    glfwMakeContextCurrent(_window);
    Resize(_width,_height);

    // load opengl functions
    //gl3wInit();

    // setup callbacks
    glfwSetMouseButtonCallback(_window, ClickCallback);
    glfwSetScrollCallback(_window, ScrollCallback);
    //glfwSetKeyCallback(_window, KeyboardCallback);
    //glfwSetCharCallback(_window, ImGui_ImplGlfw_CharCallback);
    //glfwSetCharCallback(_window, CharCallback);
    glfwSetCursorPosCallback(_window, MouseMoveCallback);
    glfwSetWindowSizeCallback(_window, ResizeCallback);

    // ui
    GetContentScale();
    SetupImgui();

    // screen space quad
    SetupScreenSpaceQuadShader();
    SetupScreenSpaceQuad();
   
  }
}
  

// window destructor
//----------------------------------------------------------------------------
Window::~Window()
{
  ClearImgui();
  
  if(_splitter)delete _splitter;
  if(_mainView)delete _mainView;
  if(_window)glfwDestroyWindow(_window);
}

// create full screen window
//----------------------------------------------------------------------------
Window* 
Window::CreateFullScreenWindow()
{
  return new Window(true);
}

// create standard window
//----------------------------------------------------------------------------
Window*
Window::CreateStandardWindow(int width, int height)
{
  return new Window(width, height);
}

// Resize
//----------------------------------------------------------------------------
void 
Window::Resize(unsigned width, unsigned height)
{
  CollectLeaves();
  if (width == _width && height == _height)
    return;
  _width = width;
  _height = height;
  if(_width <= 0 || _height <= 0)_valid = false;
  else _valid = true;
  _mainView->Resize(0, 0, _width, _height, true);
  _splitter->Resize(_width, _height);
  _splitter->RecurseBuildMap(_mainView);
}

void
Window::SetActiveView(View* view)
{
  if(_activeView)
  {
    if(_activeView == view)return;
    _activeView->ClearOver();

  }
  if(view)
  {
    _activeView = view;
    _activeView->SetOver();
  }
  else _activeView = NULL;
  
}

// split view
//----------------------------------------------------------------------------
View* 
Window::SplitView(View* view, double perc, bool horizontal, bool fixed)
{
  if(!view->IsLeaf())
  {
    std::cerr << "Can't split non-leaf view! Sorry!!!" << std::endl;
    return NULL;
  }
  view->SetLeaf();
  if(horizontal)
  {
    view->SetHorizontal();
    view->Split(perc, horizontal, fixed);
  }
  else
  {
    view->ClearHorizontal();
    view->Split(perc, horizontal, fixed);
  }
  
  view->SetPerc(perc);

  BuildSplittersMap();
  return view;
}

// collect leaves views (contains actual ui elements)
//----------------------------------------------------------------------------
void 
Window::CollectLeaves(View* view)
{
  if(view == NULL)
  {
    view = _mainView;
    _leaves.clear();
  }
  if(view->IsLeaf())_leaves.push_back(view);
  else
  {
    if(view->GetLeft())CollectLeaves(view->GetLeft());
    if(view->GetRight())CollectLeaves(view->GetRight());
  }
}

// get view (leaf) under mouse
//----------------------------------------------------------------------------
View*
Window::GetViewUnderMouse(int x, int y)
{
  for(auto leaf: _leaves)
  {
    if(leaf->Contains(x, y))return leaf;
  }
  return NULL;
}

// build split map
//----------------------------------------------------------------------------
void 
Window::BuildSplittersMap()
{
  _splitter->BuildMap(_width, _height);
  _splitter->RecurseBuildMap(_mainView);
}

// get context version infos
//----------------------------------------------------------------------------
void 
Window::GetContextVersionInfos()
{
  // get GLFW, OpenGL version And GLEW Version:
    _iOpenGLMajor = glfwGetWindowAttrib(_window, GLFW_CONTEXT_VERSION_MAJOR);
    _iOpenGLMinor = glfwGetWindowAttrib(_window, GLFW_CONTEXT_VERSION_MINOR);
    _iOpenGLRevision = glfwGetWindowAttrib(_window, GLFW_CONTEXT_REVISION);

    // print them
    std::cout << "Status: Using GLFW Version "    
      << glfwGetVersionString() << std::endl;

    std::cout << "Status: Using OpenGL Version: " << _iOpenGLMajor  << "," 
      << _iOpenGLMinor << ", Revision : " << _iOpenGLRevision << std::endl;
}

// get parent Window stored in user data
//----------------------------------------------------------------------------
Window* 
Window::GetUserData(GLFWwindow* window)
{
  return (Window*)glfwGetWindowUserPointer(window);
}

// set current context
//----------------------------------------------------------------------------
void
Window::SetContext()
{
  glfwMakeContextCurrent(_window);
}

void 
Window::GetContentScale()
{
  glfwGetWindowContentScale(_window, &_dpiX, &_dpiY);
  //void glfwGetMonitorContentScale	(	NULL,xscale, yscale); 
}

// draw
//----------------------------------------------------------------------------
void 
Window::Draw()
{
  if(!_valid)return;
  SetContext();
  // start the imgui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  
  ImGui::NewFrame();
  ImGui::SetWindowSize(pxr::GfVec2f(GetWidth(), GetHeight()));
  ImGui::SetWindowPos(pxr::GfVec2f(0,0));

  if(_mainView)_mainView->Draw();

  // draw splitters
  _splitter->Draw();

  // render the imgui frame
  ImGui::Render();
  glViewport(0, 0, (int)_io->DisplaySize.x, (int)_io->DisplaySize.y);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// setup imgui
//----------------------------------------------------------------------------
void 
Window::SetupImgui()
{
  // setup imgui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsLight();
  _io = &(ImGui::GetIO());

  // load fonts
  std::string exeFolder = GetInstallationFolder();
  std::string fontPath;
  fontPath = exeFolder + "/../../fonts/montserrat/Montserrat-Bold.otf";
  _boldFont = _io->Fonts->AddFontFromFileTTF(
    fontPath.c_str(),
    _fontSize, 
    NULL, 
    _io->Fonts->GetGlyphRangesDefault()
  );

  fontPath = exeFolder + "/../../fonts/montserrat/Montserrat-Medium.otf";
  _mediumFont = _io->Fonts->AddFontFromFileTTF(
    fontPath.c_str(),
    _fontSize, 
    NULL, 
    _io->Fonts->GetGlyphRangesDefault()
  );

  fontPath = exeFolder + "/../../fonts/montserrat/Montserrat-Regular.otf";
  _regularFont = _io->Fonts->AddFontFromFileTTF(
    fontPath.c_str(),
    _fontSize, 
    NULL, 
    _io->Fonts->GetGlyphRangesDefault()
  );
  
  // setup imgui style
  ImGui::StyleColorsAmina(NULL);
  ImGuiStyle& style = ImGui::GetStyle();
  style.Alpha = 1.f;      
  style.WindowRounding = 0.0f;
  style.ChildRounding = 0.0f;
  style.FrameRounding = 0.0f;
  style.GrabRounding = 0.0f;
  style.PopupRounding = 0.0f;
  style.ScrollbarRounding = 0.0f;
  style.TabRounding = 0.0f;
  style.WindowPadding = pxr::GfVec2f(0,0);
  style.FramePadding = pxr::GfVec2f(0,0);

  ImGui::SetNextWindowBgAlpha(1.f);

  // setup platform/renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(_window, false);
  ImGui_ImplOpenGL3_Init("#version 120");

  ImNodes::Initialize();
}

// clear imgui
//----------------------------------------------------------------------------
void 
Window::ClearImgui()
{
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImNodes::Shutdown();
  ImGui::DestroyContext();
}

bool Window::UpdateActiveTool(int mouseX, int mouseY)
{
  if(_activeTool == JVR_TOOL_DRAG)
  {
    if(_activeView)
    {
      _activeView->GetPercFromMousePosition(mouseX, mouseY);
      _mainView->Resize(0, 0, _width, _height, false);
      _splitter->Resize(_width, _height);
      _splitter->RecurseBuildMap(_mainView);
    }
  }
}

void Window::MainLoop()
{
  // Enable the OpenGL context for the current window

  while(!glfwWindowShouldClose(_window))
  {
    //glfwWaitEventsTimeout(1.0/60.0);
    //glfwWaitEvents();
    glfwPollEvents();
    glClearColor(0.3f,0.3f, 0.3f,1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // If you press escape the window will close
    if (glfwGetKey(_window, GLFW_KEY_ESCAPE))
    {
      glfwSetWindowShouldClose(_window, true);
    }
    else if(glfwGetKey(_window, GLFW_KEY_SPACE))
    {
      if(!_debounce)
      {
        _debounce = true;
      }
    }

    Draw();
    glfwSwapBuffers(_window);
  }
}

// pick splitter
//----------------------------------------------------------------------------
bool 
Window::PickSplitter(double mouseX, double mouseY)
{
  int splitterIndex = _splitter->Pick(mouseX, mouseY);
  if(splitterIndex >= 0)
  {
    _activeView = _splitter->GetViewByIndex(splitterIndex);
    if (_activeView->IsHorizontal())
      _splitter->SetVerticalCursor();
    else 
      _splitter->SetHorizontalCursor();
    return true;
  }
  _splitter->SetDefaultCursor();
  return false;
}

// keyboard event callback
//----------------------------------------------------------------------------
void 
KeyboardCallback(
  GLFWwindow* window, 
  int key, 
  int scancode, 
  int action, 
  int mods
)
{
  Window* parent = (Window*)glfwGetWindowUserPointer(window);

  //ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
  //ImGui_ImplGlfw_KeyCallback(window_in,key,scancode,action,mods);
  //if (ImGui::GetIO().WantCaptureKeyboard) return;
  if(action == GLFW_RELEASE)
  {
    parent->SetDebounce(false);
  }
  //if (action == GLFW_PRESS)
  //{
  //  /* call tutorial keyboard handler */
  //  //device_key_pressed(key);
//
  //  if (mods & GLFW_MOD_CONTROL)
  //  {
  //    switch (key) {
  //    case GLFW_KEY_UP        :  break;
  //    case GLFW_KEY_DOWN      :  break;
  //    case GLFW_KEY_LEFT      :  break;
  //    case GLFW_KEY_RIGHT     :  break;
  //    }
  //  }
  //  else
  //  {
  //    switch (key) 
  //    {
  //    /*
  //    case GLFW_KEY_LEFT      : app->camera.rotate(-0.02f,0.0f); break;
  //    case GLFW_KEY_RIGHT     : app->camera.rotate(+0.02f,0.0f); break;
  //    case GLFW_KEY_UP        : app->camera.move(0.0f,0.0f,+speed); break;
  //    case GLFW_KEY_DOWN      : app->camera.move(0.0f,0.0f,-speed); break;
  //    case GLFW_KEY_PAGE_UP   : app->speed *= 1.2f; break;
  //    case GLFW_KEY_PAGE_DOWN : app->speed /= 1.2f; break;
//
  //    case GLFW_KEY_W : ; break;
  //    case GLFW_KEY_S : moveDelta.z = -1.0f; break;
  //    case GLFW_KEY_A : moveDelta.x = -1.0f; break;
  //    case GLFW_KEY_D : moveDelta.x = +1.0f; break;
  //    */
  //    case GLFW_KEY_F :
  //      std::cout << "KEY F" << std::endl;
  //      glfwDestroyWindow(window);
  //      if (parent->IsFullScreen()) {
  //        parent = parent->CreateStandardWindow(
  //          parent->GetWidth(),parent->GetHeight());
  //      }
  //      else {
  //        parent = parent->CreateFullScreenWindow();
  //      }
  //      window = parent->GetWindow();
  //      glfwMakeContextCurrent(window);
  //      glfwSetWindowUserPointer(window, parent);
  //      parent->SetFullScreen(!parent->IsFullScreen());
  //      //app->_fullscreen = !app->_fullscreen;
  //      break;
  //      
  //    /*  
  //    case GLFW_KEY_C : std::cout << camera.str() << std::endl; break;
  //    case GLFW_KEY_HOME: g_debug=clamp(g_debug+0.01f); PRINT(g_debug); break;
  //    case GLFW_KEY_END : g_debug=clamp(g_debug-0.01f); PRINT(g_debug); break;
  //      
  //    case GLFW_KEY_SPACE: {
  //      Ref<Image> image = new Image4uc(width, height, (Col4uc*)pixels, true, "", true);
  //      storeImage(image, "screenshot.tga");
  //      break;
  //    }
  //      
  //    case GLFW_KEY_ESCAPE:
  //    */
  //    case GLFW_KEY_Q: 
  //    std::cout << "KEY Q" << std::endl;
  //      glfwSetWindowShouldClose(window,1);
  //      break;
  //    }
  //  }
  //}
  //else if (action == GLFW_RELEASE)
  //{
  //  std::cout << "KEY RELEASE" << std::endl;
  //  parent->SetDebounce(false);
  //  
  //  switch (key)
  //  {
  //  case GLFW_KEY_W : moveDelta.z = 0.0f; break;
  //  case GLFW_KEY_S : moveDelta.z = 0.0f; break;
  //  case GLFW_KEY_A : moveDelta.x = 0.0f; break;
  //  case GLFW_KEY_D : moveDelta.x = 0.0f; break;
  //  }
  //  
  //}
}

void 
ClickCallback(GLFWwindow* window, int button, int action, int mods)
{ 
  Window* parent = Window::GetUserData(window);
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  if (action == GLFW_RELEASE)
  {
    parent->SetActiveTool(JVR_TOOL_NONE);
    if(parent->GetActiveView())
    {
      parent->GetActiveView()->MouseButton(button, action, mods);
    }
  }
  else if (action == GLFW_PRESS)
  {
    double x,y;
    glfwGetCursorPos(window,&x,&y);
    View* view = parent->GetViewUnderMouse((int)x, (int)y);
    if(view) 
    {
      parent->SetActiveView(view);
    }

    if(parent->PickSplitter(x, y))
    {
      parent->SetActiveTool(JVR_TOOL_DRAG);
    }
    else if(parent->GetActiveView())
    {
      parent->GetActiveView()->MouseButton(button, action, mods);
    }
  }
  parent->Draw();
}

void 
ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  Window* parent = Window::GetUserData(window);
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
  if(parent->GetActiveView())
    parent->GetActiveView()->MouseWheel(xoffset, yoffset);
  parent->Draw();
}

void 
CharCallback(GLFWwindow* window, unsigned c)
{
  //ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);
}

void 
MouseMoveCallback(GLFWwindow* window, double x, double y)
{
  //if (ImGui::GetIO().WantCaptureMouse) return;

  Window* parent = Window::GetUserData(window);
  View* view = parent->GetViewUnderMouse((int)x, (int)y);
  parent->PickSplitter(x, y);
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  if(parent->GetActiveTool() != JVR_TOOL_NONE)
  {
    parent->UpdateActiveTool(x, y);
  }
  else
  {
    if(parent->GetActiveView())
    {
      parent->GetActiveView()->MouseMove(x, y);
    }
  }

}

void 
DisplayCallback(GLFWwindow* window)
{
  Window* parent = (Window*)glfwGetWindowUserPointer(window);
  
  //std::cout << "DISPLAY !!!" << std::endl;
  /*
  // update camera 
  camera.move(moveDelta.x*speed, moveDelta.y*speed, moveDelta.z*speed);
  ISPCCamera ispccamera = camera.getISPCCamera(width,height,true);
    if (print_camera)
    std::cout << camera.str() << std::endl;

  // render image using ISPC
  initRayStats();
  double t0 = getSeconds();
  render(pixels,width,height,float(time0-t0),ispccamera);
  double dt0 = getSeconds()-t0;
  avg_render_time.add(dt0);
  double mrayps = double(getNumRays())/(1000000.0*dt0);
  avg_mrayps.add(mrayps);

  // draw pixels to screen 
  glDrawPixels(width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels);

  ImGui_ImplGlfwGL3_NewFrame();
  
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  //window_flags |= ImGuiWindowFlags_NoScrollbar;
  //window_flags |= ImGuiWindowFlags_MenuBar;
  //window_flags |= ImGuiWindowFlags_NoMove;
  //window_flags |= ImGuiWindowFlags_NoResize;
  //window_flags |= ImGuiWindowFlags_NoCollapse;
  //window_flags |= ImGuiWindowFlags_NoNav;
  
  //ImGui::GetStyle().WindowBorderSize = 0.0f;
  //ImGui::SetNextWindowPos(ImVec2(width-200,0));
  //ImGui::SetNextWindowSize(ImVec2(200,height));
  ImGui::SetNextWindowBgAlpha(0.3f);
  ImGui::Begin("Embree", nullptr, window_flags);
  drawGUI();
  ImGui::Text("%3.2f fps",1.0f/avg_render_time.get());
#if defined(RAY_STATS)
  ImGui::Text("%3.2f Mray/s",avg_mrayps.get());
#endif
  ImGui::End();
    
  //ImGui::ShowDemoWindow();
      
  ImGui::Render();
  ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
  */
  /*
  glClearColor(
    (float)rand()/(float)RAND_MAX,
    (float)rand()/(float)RAND_MAX,
    (float)rand()/(float)RAND_MAX,
    1.f
  );
  */

  /*
  double dt1 = embree::getSeconds()-t0;
  avg_frame_time.add(dt1);

  if (print_frame_rate)
  {
    std::ostringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(2);
    stream << "render: ";
    stream << 1.0f/dt0 << " fps, ";
    stream << dt0*1000.0f << " ms, ";
#if defined(RAY_STATS)
    stream << mrayps << " Mray/s, ";
#endif
    stream << "display: ";
    stream << 1.0f/dt1 << " fps, ";
    stream << dt1*1000.0f << " ms, ";
    stream << width << "x" << height << " pixels";
    std::cout << stream.str() << std::endl;
  } 
  */
}

void 
ResizeCallback(GLFWwindow* window, int width, int height)
{
  Window* parent = (Window*)glfwGetWindowUserPointer(window);
  parent->Resize(width, height);
  glViewport(0, 0, width, height);
}

JVR_NAMESPACE_CLOSE_SCOPE