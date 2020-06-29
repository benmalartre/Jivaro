#include "../utils/glutils.h"
#include "../utils/files.h"
#include "../utils/icons.h"
#include "../utils/utils.h"
#include "../widgets/dummy.h"
#include "../widgets/viewport.h"
#include "../app/application.h"
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/base/arch/systemInfo.h>
#include "window.h"
#include "view.h"
#include "splitter.h"

AMN_NAMESPACE_OPEN_SCOPE

ImFontAtlas* AMN_SHARED_ATLAS = NULL;
ImFont* AMN_BOLD_FONTS[3] = { NULL, NULL, NULL };
ImFont* AMN_MEDIUM_FONTS[3] = { NULL, NULL, NULL };
ImFont* AMN_REGULAR_FONTS[3] = { NULL, NULL, NULL };

//
// Shared Font Atlas
//
void AMNCreateFontAtlas()
{
  static float fontSizes[3] = { 16.f,32.f,64.f };
  AMN_SHARED_ATLAS = new ImFontAtlas();

  // load fonts
  std::string exeFolder = GetInstallationFolder();

  std::string fontPath;
  for (int i = 0; i < 3; ++i) {
    fontPath = exeFolder + "/../../fonts/montserrat/Montserrat-Bold.otf";
    AMN_BOLD_FONTS[i] = AMN_SHARED_ATLAS->AddFontFromFileTTF(
      fontPath.c_str(),
      fontSizes[i],
      NULL,
      AMN_SHARED_ATLAS->GetGlyphRangesDefault()
    );

    fontPath = exeFolder + "/../../fonts/montserrat/Montserrat-Medium.otf";
    AMN_MEDIUM_FONTS[i] = AMN_SHARED_ATLAS->AddFontFromFileTTF(
      fontPath.c_str(),
      fontSizes[i],
      NULL,
      AMN_SHARED_ATLAS->GetGlyphRangesDefault()
    );

    fontPath = exeFolder + "/../../fonts/montserrat/Montserrat-Regular.otf";
    AMN_REGULAR_FONTS[i] = AMN_SHARED_ATLAS->AddFontFromFileTTF(
      fontPath.c_str(),
      fontSizes[i],
      NULL,
      AMN_SHARED_ATLAS->GetGlyphRangesDefault()
    );
  }
}

void AMNDeleteFontAtlas()
{
  if (AMN_SHARED_ATLAS)delete AMN_SHARED_ATLAS;
}

// generic miniDart theme
void AMNStyle(ImGuiStyle* dst)
{
  ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
  ImVec4* colors = style->Colors;

  style->Alpha = 1.f;
  style->WindowRounding = 0.0f;
  style->ChildRounding = 0.0f;
  style->FrameRounding = 0.0f;
  style->GrabRounding = 0.0f;
  style->PopupRounding = 0.0f;
  style->ScrollbarRounding = 0.0f;
  style->TabRounding = 0.0f;
  style->WindowPadding = pxr::GfVec2f(0, 0);
  style->FramePadding = pxr::GfVec2f(0, 0);

  style->WindowRounding = 2.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows
  style->ScrollbarRounding = 3.0f;             // Radius of grab corners rounding for scrollbar
  style->GrabRounding = 2.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
  style->AntiAliasedLines = true;
  style->AntiAliasedFill = true;
  style->WindowRounding = 2;
  style->ChildRounding = 2;
  //style->ScrollbarSize = 16;
  style->ScrollbarRounding = 3;
  style->GrabRounding = 2;
  style->ItemSpacing.x = 2;
  style->ItemSpacing.y = 2;
  style->IndentSpacing = 12;
  style->TabRounding = 2;
  //style->FramePadding.x = 0;
  //style->FramePadding.y = 0;
  //style->Alpha = 1.0f;
  style->FrameRounding = 3.0f;


  colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
  //colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.93f, 0.93f, 0.93f, 0.98f);
  colors[ImGuiCol_Border] = ImVec4(0.71f, 0.71f, 0.71f, 0.08f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.71f, 0.71f, 0.55f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.71f, 0.78f, 0.69f, 0.98f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.82f, 0.78f, 0.78f, 0.51f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.61f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.90f, 0.90f, 0.90f, 0.30f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.92f, 0.92f, 0.92f, 0.78f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.184f, 0.407f, 0.193f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.650f, 0.650f, 0.650f, 0.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.502f, 0.502f, 0.502f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.703f, 0.703f, 0.703f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.71f, 0.78f, 0.69f, 0.31f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.71f, 0.78f, 0.69f, 0.80f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.71f, 0.78f, 0.69f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  colors[ImGuiCol_NavHighlight] = colors[ImGuiCol_HeaderHovered];
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
  colors[ImGuiCol_Tab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.62f, 0.62f, 0.62f, 1.00f);
}

// fullscreen window constructor
//----------------------------------------------------------------------------
Window::Window(bool fullscreen, const std::string& name) :
_pixels(NULL), _debounce(0),_mainView(NULL), _activeView(NULL), 
_pickImage(0),_splitter(NULL),_fontSize(16.f), _name(name),_forceRedraw(0)
{
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  glfwWindowHint(GLFW_RED_BITS,mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS,mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS,mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE,mode->refreshRate);
  //glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);

  //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_STENCIL_BITS, 8);

  _window = glfwCreateWindow(mode->width, mode->height, "AMINA.0.0",  monitor, NULL);
  _width = mode->width;
  _height = mode->height;
  _shared = true;

}

// width/height window constructor
//----------------------------------------------------------------------------
Window::Window(int width, int height, const std::string& name):
_pixels(NULL), _debounce(0),_mainView(NULL), _activeView(NULL), 
_pickImage(0), _splitter(NULL),_fontSize(16.f), _name(name),_forceRedraw(0)
{
  _width = width;
  _height = height;
  _shared = true;
  //glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_STENCIL_BITS, 8);
  
  _window = glfwCreateWindow(_width,_height,"AMINA.0.0",NULL,NULL);

}

// child window constructor
//----------------------------------------------------------------------------
Window::Window(int width, int height, GLFWwindow* parent, const std::string& name) :
  _pixels(NULL), _debounce(0), _mainView(NULL), _activeView(NULL),
  _pickImage(0), _splitter(NULL), _fontSize(16.f), _name(name),_forceRedraw(0)
{
  _width = width;
  _height = height;
  _shared = false;
  //glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_FLOATING, true);

  _window = glfwCreateWindow(_width, _height, "CHILD", NULL, parent);
}

// initialize
//----------------------------------------------------------------------------
void 
Window::Init(Application* app)
{
  _app = app;
  if(_window)
  {
    // window datas
    glfwSetWindowUserPointer(_window, this);

    // set current opengl context
    glfwMakeContextCurrent(_window);

    if (_shared) {
      GetContextVersionInfos();
      // load opengl functions
      pxr::GlfGlewInit();
      pxr::GlfSharedGLContextScopeHolder sharedContext;
      pxr::GlfContextCaps::InitInstance();
      pxr::GlfContextCaps const& caps = pxr::GlfContextCaps::GetInstance();

      AMNCreateFontAtlas();
      AMNInitializeIcons();
    }

    // setup callbacks
    glfwSetWindowSizeCallback(_window, ResizeCallback);
    glfwSetMouseButtonCallback(_window, ClickCallback);
    glfwSetScrollCallback(_window, ScrollCallback);
    glfwSetKeyCallback(_window, KeyboardCallback);
    glfwSetCharCallback(_window, CharCallback);
    glfwSetCursorPosCallback(_window, MouseMoveCallback);

    // create main splittable view
    _mainView = new View(NULL, pxr::GfVec2f(0,0), pxr::GfVec2f(_width, _height));
    _mainView->SetWindow(this);
    _splitter = new Splitter();

    Resize(_width, _height);

    // ui
    SetupImgui();
   
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
  return new Window(true, "Amina");
}

// create standard window
//----------------------------------------------------------------------------
Window*
Window::CreateStandardWindow(int width, int height)
{
  return new Window(width, height, "Amina");
}

// child window
//----------------------------------------------------------------------------
Window*
Window::CreateChildWindow(int width, int height, GLFWwindow* parent)
{
  return new Window(width, height, parent, "Child");
}

void 
Window::AddChild(Window* child)
{
  _childrens.push_back(child);
}

void 
Window::RemoveChild(Window* child)
{
  for (int i = 0; i < _childrens.size(); ++i) {
    if (_childrens[i] == child) {
      _childrens.erase(_childrens.begin() + i);
      break;
    }
  }
}

// Resize
//----------------------------------------------------------------------------
void 
Window::Resize(unsigned width, unsigned height)
{
  CollectLeaves(_mainView);
  if (width == _width && height == _height && _pixels)
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
  if (_activeView)
  {
    if (_activeView == view)return;
    _activeView->ClearFlag(View::OVER);
    _activeView->ClearFlag(View::ACTIVE);
    _activeView->ClearFlag(View::INTERACTING);
    _activeView->SetDirty();
  }
  if(view)
  {
    _activeView = view;
    _activeView->SetFlag(View::OVER);
    _activeView->SetFlag(View::ACTIVE);
    _activeView->SetDirty();
  }
  else _activeView = NULL;
  
}

// split view
//----------------------------------------------------------------------------
View* 
Window::SplitView(View* view, double perc, bool horizontal, int fixed, int numPixels)
{
  if(!view->GetFlag(View::LEAF))
  {
    std::cerr << "Can't split non-leaf view! Sorry!!!" << std::endl;
    return NULL;
  }
  view->SetFlag(View::LEAF);
  if(horizontal)
  {
    view->SetFlag(View::HORIZONTAL);
    float height = (view->GetMax()[1] - view->GetMin()[1]);
    if(fixed & View::LFIXED && numPixels > 0)
      view->Split(numPixels / height, horizontal, fixed, numPixels);
    else if(fixed & View::RFIXED && numPixels > 0)
      view->Split((height-numPixels) / height, horizontal, fixed, numPixels);
    else
      view->Split(perc, horizontal, fixed, numPixels);
  }
  else
  {
    view->ClearFlag(View::HORIZONTAL);
    float width = (view->GetMax()[0] - view->GetMin()[0]);
    if (fixed & View::LFIXED && numPixels > 0)
      view->Split(numPixels/width, horizontal, fixed, numPixels);
    else if (fixed & View::RFIXED && numPixels > 0)
      view->Split((width-numPixels)/width, horizontal, fixed, numPixels);
    else
      view->Split(perc, horizontal, fixed, numPixels);
  }
  
  //view->SetPerc(perc);

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
  if (view->GetFlag(View::LEAF)) {
    _leaves.push_back(view);
    view->SetDirty();
  }
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
  return (Window*)(glfwGetWindowUserPointer(window));
}

// set current context
//----------------------------------------------------------------------------
void
Window::SetGLContext()
{
  glfwMakeContextCurrent(_window);
  ImGui::SetCurrentContext(_context);
}

// draw
//----------------------------------------------------------------------------
void 
Window::Draw()
{
  if(!_valid)return;
  SetGLContext();
  GLCheckError("### WINDOW 0");
  // start the imgui frame
  ImGui_ImplOpenGL3_NewFrame();
  GLCheckError("### WINDOW 1");
  ImGui_ImplGlfw_NewFrame(_window);
  GLCheckError("### WINDOW 2");
  
  ImGui::NewFrame();
  GLCheckError("### WINDOW 3");
  ImGui::SetWindowSize(pxr::GfVec2f(GetWidth(), GetHeight()));
  ImGui::SetWindowPos(pxr::GfVec2f(0,0));
  if (_mainView)_mainView->Draw( _forceRedraw > 0 );
  _forceRedraw = pxr::GfMax(0, _forceRedraw-1);
  GLCheckError("### WINDOW 4");

  // draw splitters
  _splitter->Draw();

  GLCheckError("### WINDOW 5");
  // render the imgui frame
  ImGui::Render();
  GLCheckError("### WINDOW 6");
  glViewport(0, 0, (int)_io->DisplaySize.x, (int)_io->DisplaySize.y);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  GLCheckError("### WINDOW 7");
}

// setup imgui
//----------------------------------------------------------------------------
void 
Window::SetupImgui()
{
  static float fontSizes[3] = { 16.f,32.f,64.f };
  // setup imgui context
  IMGUI_CHECKVERSION();
  _context = ImGui::CreateContext(AMN_SHARED_ATLAS);
  ImGui::SetCurrentContext(_context);
  
  _io = &(ImGui::GetIO());
  //_io->FontAllowUserScaling = true;
  /*
  // load fonts
  std::string exeFolder = GetInstallationFolder();
  std::string fontPath;
  for (int i = 0; i < 3; ++i) {
    fontPath = exeFolder + "/../../fonts/montserrat/Montserrat-Bold.otf";
    _boldFont[i] = _io->Fonts->AddFontFromFileTTF(
      fontPath.c_str(),
      fontSizes[i],
      NULL,
      _io->Fonts->GetGlyphRangesDefault()
    );

    fontPath = exeFolder + "/../../fonts/montserrat/Montserrat-Medium.otf";
    _mediumFont[i] = _io->Fonts->AddFontFromFileTTF(
      fontPath.c_str(),
      fontSizes[i],
      NULL,
      _io->Fonts->GetGlyphRangesDefault()
    );

    fontPath = exeFolder + "/../../fonts/montserrat/Montserrat-Regular.otf";
    _regularFont[i] = _io->Fonts->AddFontFromFileTTF(
      fontPath.c_str(),
      fontSizes[i],
      NULL,
      _io->Fonts->GetGlyphRangesDefault()
    );
  }
  */
  // setup imgui style
  //ImGui::StyleColorsLight();
  //ImGui::StyleColorsClassic();
  ImGuiStyle& style = ImGui::GetStyle();
  AMNStyle(&style);

  // setup platform/renderer bindings
  if (_shared) {
    pxr::GlfContextCaps const & caps = pxr::GlfContextCaps::GetInstance();
    ImGui_ImplGlfw_InitForOpenGL(_window, false);
    if (caps.glslVersion < 330) {
      ImGui_ImplOpenGL3_Init("#version 120 ");
    }
    else {
      ImGui_ImplOpenGL3_Init("#version 330 ");
    }
  }
}

// clear imgui
//----------------------------------------------------------------------------
void 
Window::ClearImgui()
{
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(_context);
}

bool Window::UpdateActiveTool(int x, int y)
{
  if(_activeTool == AMN_TOOL_SPLITTER)
  {
    if(_activeView)
    {
      _activeView->GetPercFromMousePosition(x, y);
      _mainView->Resize(0, 0, _width, _height, false);
      _splitter->Resize(_width, _height);
      _splitter->RecurseBuildMap(_mainView);
    }
  }
  else if(_activeTool == AMN_TOOL_CAMERA)
  {
    if(GetActiveView())
    {
      GetActiveView()->MouseMove(x, y);
    }
  }
  return false;
}

void Window::MainLoop()
{
  while(!glfwWindowShouldClose(_window))
  {
    SetGLContext();
    //glfwWaitEventsTimeout(1.0/60.0);
    glfwPollEvents();
    if(_app->IsPlaying())_app->PlayBack();
    else _app->Update();

    // main window
    Draw();
    glfwSwapBuffers(_window);

    // child windows
    for (auto& child : _childrens) {
      if (glfwGetWindowAttrib(child->GetGlfwWindow(), GLFW_FOCUSED))
      {
        child->Draw();
        glfwSwapBuffers(child->GetGlfwWindow());
      }
    }
    
    _app->ComputeFramerate(glfwGetTime());
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
    if (_activeView->GetFlag(View::HORIZONTAL))
      _splitter->SetVerticalCursor();
    else 
      _splitter->SetHorizontalCursor();
    return true;
  }
  else _splitter->SetDefaultCursor();
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
  Application* app = parent->GetApplication();
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
  if(action == GLFW_RELEASE)
  {
    parent->SetDebounce(false);
  }
  if (action == GLFW_PRESS)
  {
    switch(key)
    {
  
      case GLFW_KEY_SPACE:
      {
        if(app->IsPlaying())
        {
          app->SetLoop(false);
          app->StopPlayBack();
        }
        else
        {
          app->SetLoop(true);
          app->StartPlayBack();
        }
        break;
      }
      case GLFW_KEY_LEFT:
      {
        app->PreviousFrame();
        break;
      }

      case GLFW_KEY_RIGHT:
      {
        app->NextFrame();
        break;
      }

      case GLFW_KEY_ESCAPE:
      {
        glfwSetWindowShouldClose(window, true);
        break;
      }

      default:
      {
        View* view = parent->GetActiveView();
        if (view)
        {
          view->Keyboard(key, scancode, action, mods);
          view->SetDirty();
        }
      }
    }
  }
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
  ImGui::SetCurrentContext(parent->GetContext());

  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

  if (action == GLFW_RELEASE)
  {
    parent->SetActiveTool(AMN_TOOL_NONE);
    View* view = parent->GetActiveView();
    if(view)
    {
      view->MouseButton(button, action, mods);
      view->ClearFlag(View::INTERACTING);
      view->SetDirty();
    }
  }
  else if (action == GLFW_PRESS)
  {
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    switch (parent->GetActiveTool()) {
    case AMN_TOOL_MENU:
      parent->GetActiveView()->MouseButton(button, action, mods);
      break;

    case AMN_TOOL_SPLITTER:
      break;

    default:
      if (parent->PickSplitter(x, y))
      {
        parent->SetActiveTool(AMN_TOOL_SPLITTER);
      }
      else
      {
        View* view = parent->GetViewUnderMouse((int)x, (int)y);
        if (view) {
          parent->SetActiveView(view);
          view->SetFlag(View::INTERACTING);
          view->MouseButton(button, action, mods);
        }
      }
      break;
    }
  }
}

void 
ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  Window* parent = Window::GetUserData(window);
  ImGui::SetCurrentContext(parent->GetContext());
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
  if(parent->GetActiveView())
    parent->GetActiveView()->MouseWheel(xoffset, yoffset);
}

void 
CharCallback(GLFWwindow* window, unsigned c)
{
  ImGui_ImplGlfw_CharCallback(window, c);
}

void 
MouseMoveCallback(GLFWwindow* window, double x, double y)
{
  Window* parent = Window::GetUserData(window);
  ImGui::SetCurrentContext(parent->GetContext());
  View* view = parent->GetViewUnderMouse((int)x, (int)y);
  View* active = parent->GetActiveView();
  if(parent->GetActiveTool() != AMN_TOOL_NONE)
  {
    parent->UpdateActiveTool(x, y);
  }
  else
  {
    if (active && active->GetFlag(View::INTERACTING))
      active->MouseMove(x, y);
    else {
      if (parent->PickSplitter(x, y))return;
      else if (view) {
        parent->SetActiveView(view);
        parent->GetActiveView()->MouseMove(x, y);
      }
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
  Window* parent = (Window*)glfwGetWindowUserPointer(window);
  parent->Resize(width, height);
  glViewport(0, 0, width, height);
}

AMN_NAMESPACE_CLOSE_SCOPE