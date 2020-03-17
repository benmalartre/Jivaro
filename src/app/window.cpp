#include "window.h"
#include "view.h"
#include "splitter.h"
#include "../utils/glutils.h"
#include "../widgets/dummy.h"
#include "../widgets/viewport.h"
#include "../imgui/imgui_custom.h"
#include "../imgui/imgui_test.h"

AMN_NAMESPACE_OPEN_SCOPE

// fullscreen window constructor
//----------------------------------------------------------------------------
AmnWindow::AmnWindow(bool fullscreen) :
_pixels(NULL), _debounce(0),_mainView(NULL), _activeView(NULL), 
_pickImage(0),_cursor(NULL),_splitter(NULL)
{
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  glfwWindowHint(GLFW_RED_BITS,mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS,mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS,mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE,mode->refreshRate);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  _window = glfwCreateWindow(mode->width, mode->height, "AMINA.0.0",  monitor, NULL);
  _width = mode->width;
  _height = mode->height;
  
  Init();
}

// width/height window constructor
//----------------------------------------------------------------------------
AmnWindow::AmnWindow(int width, int height):
_pixels(NULL), _debounce(0),_mainView(NULL), _activeView(NULL), 
_pickImage(0),_cursor(NULL), _splitter(NULL)
{
  _width = width;
  _height = height;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  
  _window = glfwCreateWindow(_width,_height,"AMINA.0.0",NULL,NULL);
  Init();
}

// initialize
//----------------------------------------------------------------------------
void 
AmnWindow::Init()
{
  if(_window)
  {
    // create main splittable view
    _mainView = new AmnView(NULL, pxr::GfVec2f(0,0), pxr::GfVec2f(_width, _height));
    _mainView->SetWindow(this);
    _splitter = new AmnSplitter();
    
    // window datas
    GetContextVersionInfos();
    glfwSetWindowUserPointer(_window, this);
    
    // set current opengl context
    glfwMakeContextCurrent(_window);
    Resize(_width,_height);

    // load opengl functions
    gl3wInit();

    // setup callbacks
    glfwSetMouseButtonCallback(_window, ClickCallback);
    //glfwSetScrollCallback(_window, ImGui_ImplGlfw_ScrollCallback);
    glfwSetScrollCallback(_window, ScrollCallback);
    glfwSetKeyCallback(_window, KeyboardCallback);
    //glfwSetCharCallback(_window, ImGui_ImplGlfw_CharCallback);
    glfwSetCharCallback(_window, CharCallback);
    glfwSetCursorPosCallback(_window, MouseMoveCallback);
    glfwSetWindowSizeCallback(_window, ResizeCallback);

    // imgui
    SetupImgui();
    GLUIInit();

    // screen space quad
    ScreenSpaceQuad();
  }
}

void AmnWindow::DummyFill()
{
  for(auto leaf: _leaves)
  {
    std::cout << leaf->GetText() << std::endl;
    //AmnUI* ui = new DummyUI(leaf, leaf->GetName()+":content");
    AmnUI* ui = new AmnViewportUI(leaf, EMBREE);
    leaf->SetContent(ui);
  }
}
  

// window destructor
//----------------------------------------------------------------------------
AmnWindow::~AmnWindow()
{
  GLUITerm();
  ClearImgui();
  
  if(_splitter)delete _splitter;
  if(_mainView)delete _mainView;
  if(_window)glfwDestroyWindow(_window);
}

// create full screen window
//----------------------------------------------------------------------------
AMN_EXPORT AmnWindow* 
AmnWindow::CreateFullScreenWindow()
{
  return new AmnWindow(true);
}

// create standard window
//----------------------------------------------------------------------------
AMN_EXPORT AmnWindow*
AmnWindow::CreateStandardWindow(int width, int height)
{
  return new AmnWindow(width, height);
}

// Resize
//----------------------------------------------------------------------------
void 
AmnWindow::Resize(unsigned width, unsigned height)
{

  if (width == _width && height == _height && _pixels)
    return;
  _width = width;
  _height = height;
  _splitter->Resize(_width, _height, _mainView);
}

// split view
//----------------------------------------------------------------------------
void 
AmnWindow::SplitView(AmnView* view, unsigned perc, bool horizontal )
{
  if(!view->IsLeaf())
  {
    std::cerr << "Can't split non-leaf view! Sorry!!!" << std::endl;
    return;
  }
  view->SetLeaf();
  view->SetPerc(perc);
  if(horizontal)
  {
    view->SetHorizontal();
    view->Split();
  }
  else
  {
    view->ClearHorizontal();
    view->Split();
  }
  BuildSplittersMap();
}

// collect leaves views (contains actual ui elements)
//----------------------------------------------------------------------------
void 
AmnWindow::CollectLeaves(AmnView* view)
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
AmnView*
AmnWindow::GetViewUnderMouse(int x, int y)
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
AmnWindow::BuildSplittersMap()
{
  _splitter->BuildMap(_mainView);
}

// get context version infos
//----------------------------------------------------------------------------
void 
AmnWindow::GetContextVersionInfos()
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

// get parent AmnWindow stored in user data
//----------------------------------------------------------------------------
AmnWindow* 
AmnWindow::GetUserData(GLFWwindow* window)
{
  return static_cast<AmnWindow*>(glfwGetWindowUserPointer(window));
}

// set current context
//----------------------------------------------------------------------------
void
AmnWindow::SetContext()
{
  glfwMakeContextCurrent(_window);
}

// draw
//----------------------------------------------------------------------------
void 
AmnWindow::Draw()
{
  SetContext();
  // start the imgui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  
  ImGui::NewFrame();
  ImGui::SetWindowSize(pxr::GfVec2f(GetWidth(), GetHeight()));
  ImGui::SetWindowPos(pxr::GfVec2f(0,0));

  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowPadding = pxr::GfVec2f(0,0);
  style.FramePadding = pxr::GfVec2f(0,0);

  if(_mainView)_mainView->Draw();

  // render the imgui frame
  ImGui::Render();
  glViewport(0, 0, (int)_io->DisplaySize.x, (int)_io->DisplaySize.y);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // draw splitters
  _splitter->Draw();

}

// draw pick image
//----------------------------------------------------------------------------
void 
AmnWindow::DrawPickImage()
{
  int ID = 0;
  glUseProgram(EMBREE_CTXT->_screenSpaceQuadPgm);
  CreateOpenGLTexture(
    _splitter->GetWidth(),
    _splitter->GetHeight(),
    _splitter->GetPixels(),
    _pickImage,
    ID
  );
  
  glUniform1i(glGetUniformLocation(EMBREE_CTXT->_screenSpaceQuadPgm,"tex"),ID);
  DrawScreenSpaceQuad();
}

void 
AmnWindow::ScreenSpaceQuad()
{
  SetupScreenSpaceQuadShader();
  SetupScreenSpaceQuad();
}

// setup imgui
//----------------------------------------------------------------------------
void 
AmnWindow::SetupImgui()
{
  // detup imgui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  _io = &(ImGui::GetIO());
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

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

  ImGui::SetNextWindowBgAlpha(1.f);

  // setup platform/renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(_window, glfwGetCurrentContext());
  ImGui_ImplOpenGL3_Init("#version 330");

  ImNodes::Initialize();
}

// clear imgui
//----------------------------------------------------------------------------
void 
AmnWindow::ClearImgui()
{
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImNodes::Shutdown();
  ImGui::DestroyContext();
}

bool AmnWindow::UpdateActiveTool(int mouseX, int mouseY)
{
  if(_activeTool == AMN_TOOL_DRAG)
  {
    if(_activeView)
    {
      _activeView->GetPercFromMousePosition(mouseX, mouseY);
      _splitter->Resize(GetWidth(), GetHeight(), _mainView);
      DrawPickImage();
    }
  }
}

void AmnWindow::MainLoop()
{
  // Enable the OpenGL context for the current window
  _guiId = 1;

  while(!glfwWindowShouldClose(_window))
  {
    glfwWaitEvents();
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
        _guiId++;
        _debounce = true;
      }
    }
    Draw();

    //TestImgui(_guiId % 3);
    glfwSwapBuffers(_window);
  }
}

// pick splitter
//----------------------------------------------------------------------------
bool 
AmnWindow::PickSplitter(double mouseX, double mouseY)
{
  _clickX = mouseX;
  _clickY = mouseY;
  int pixelValue = _splitter->GetPixelValue(mouseX, mouseY);
  if(pixelValue)
  {
    if(_cursor) glfwDestroyCursor(_cursor);
    _activeView = _splitter->GetViewByIndex(pixelValue - 1);
    if (_activeView->IsHorizontal())
      _cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    else 
      _cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    glfwSetCursor(_window, _cursor);
    return true;
  }
  else
  {
    if(_cursor) glfwDestroyCursor(_cursor);
    _cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR );
    glfwSetCursor(_window, _cursor);
    return false;
  }
  glfwSwapBuffers(_window);
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
  AmnWindow* parent = (AmnWindow*)glfwGetWindowUserPointer(window);

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
  //      glfwDestroyAmnWindow(window);
  //      if (parent->IsFullScreen()) {
  //        parent = parent->CreateStandardAmnWindow(
  //          parent->GetWidth(),parent->GetHeight());
  //      }
  //      else {
  //        parent = parent->CreateFullScreenAmnWindow();
  //      }
  //      window = parent->GetAmnWindow();
  //      glfwMakeContextCurrent(window);
  //      glfwSetAmnWindowUserPointer(window, parent);
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
  //      glfwSetAmnWindowShouldClose(window,1);
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
  std::cout << "CLICK CALLBACK !" << std::endl;
  AmnWindow* parent = AmnWindow::GetUserData(window);

  double x,y;
  glfwGetCursorPos(window,&x,&y);
  AmnView* view = parent->GetViewUnderMouse((int)x, (int)y);
  if(view) std::cout << "VIEW UNDER MOUSE : "<< view->GetContent()->GetName() << std::endl;

  if (action == GLFW_RELEASE)
  {
    parent->SetActiveTool(AMN_TOOL_NONE);
    if(view)
    {
      view->GetContent()->MouseButton(button, action, mods);
    }
  }
  else if (action == GLFW_PRESS)
  {
    if(parent->PickSplitter(x, y))
    {
      parent->SetActiveTool(AMN_TOOL_DRAG);
    }
    else if(view)
    {
      view->GetContent()->MouseButton(button, action, mods);
    }
      /*
      if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_SHIFT)
          _mouseMode = 1;
      else if (button == GLFW_MOUSE_BUTTON_LEFT && mods == GLFW_MOD_CONTROL ) 
        _mouseMode = 3;
      else if (button == GLFW_MOUSE_BUTTON_LEFT) 
        _mouseMode = 4;
      */
  }
}

void 
ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{

}

void 
CharCallback(GLFWwindow* window, unsigned c)
{

}

void 
MouseMoveCallback(GLFWwindow* window, double x, double y)
{
  //if (ImGui::GetIO().WantCaptureMouse) return;

  AmnWindow* parent = AmnWindow::GetUserData(window);
  AmnView* view = parent->GetViewUnderMouse((int)x, (int)y);
  if(view) std::cout << "VIEW UNDER MOUSE : "<< view->GetContent()->GetName() << std::endl;

  int width, height;
  glfwGetWindowSize(window, &width, &height);
  if(parent->GetActiveTool() != AMN_TOOL_NONE)
  {
    parent->UpdateActiveTool(x, y);
  }
  else
  {
    if(view)
    {
      view->GetContent()->MouseMove(x, y);
    }
  }

}

void 
DisplayCallback(GLFWwindow* window)
{
  AmnWindow* parent = (AmnWindow*)glfwGetWindowUserPointer(window);
  
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
  
  ImGuiAmnWindowFlags window_flags = 0;
  window_flags |= ImGuiAmnWindowFlags_NoTitleBar;
  //window_flags |= ImGuiAmnWindowFlags_NoScrollbar;
  //window_flags |= ImGuiAmnWindowFlags_MenuBar;
  //window_flags |= ImGuiAmnWindowFlags_NoMove;
  //window_flags |= ImGuiAmnWindowFlags_NoResize;
  //window_flags |= ImGuiAmnWindowFlags_NoCollapse;
  //window_flags |= ImGuiAmnWindowFlags_NoNav;
  
  //ImGui::GetStyle().AmnWindowBorderSize = 0.0f;
  //ImGui::SetNextAmnWindowPos(ImVec2(width-200,0));
  //ImGui::SetNextAmnWindowSize(ImVec2(200,height));
  ImGui::SetNextAmnWindowBgAlpha(0.3f);
  ImGui::Begin("Embree", nullptr, window_flags);
  drawGUI();
  ImGui::Text("%3.2f fps",1.0f/avg_render_time.get());
#if defined(RAY_STATS)
  ImGui::Text("%3.2f Mray/s",avg_mrayps.get());
#endif
  ImGui::End();
    
  //ImGui::ShowDemoAmnWindow();
      
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

#ifdef __APPLE__
  // work around glfw issue #1334
  // https://github.com/glfw/glfw/issues/1334
  static bool macMoved = false;

  if (!macMoved) {
    int x, y;
    glfwGetWindowPos(window, &x, &y);
    glfwSetWindowPos(window, ++x, y);
    macMoved = true;
  }
#endif
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
  AmnWindow* parent = (AmnWindow*)glfwGetWindowUserPointer(window);
  //glfwGetFramebufferSize(window, &width, &height);
  parent->SetWidth(width);
  parent->SetHeight(height);
  AmnSplitter* splitter = parent->GetSplitter();
  splitter->Resize(width, height, parent->GetMainView());
  parent->DrawPickImage();
  
  glViewport(0, 0, width, height);
}

AMN_NAMESPACE_CLOSE_SCOPE