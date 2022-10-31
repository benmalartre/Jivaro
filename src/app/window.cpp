#include "../utils/glutils.h"
#include "../utils/files.h"
#include "../utils/icons.h"
#include "../utils/keys.h"
#include "../ui/style.h"
#include "../ui/utils.h"
#include "../ui/viewport.h"
#include "../ui/splitter.h"
#include "../app/application.h"
#include "../app/tools.h"
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/base/arch/systemInfo.h>

#include "../app/window.h"
#include "../app/view.h"
#include "../geometry/shape.h"


JVR_NAMESPACE_OPEN_SCOPE

bool LEGACY_OPENGL = false;

int MAPPED_KEYS[GLFW_KEY_LAST + 1];
bool KEY_MAP_INITIALIZED = false;


// fullscreen window constructor
//----------------------------------------------------------------------------
Window::Window(bool fullscreen, const std::string& name) :
  _pixels(NULL), _debounce(0),_mainView(NULL), _activeView(NULL), _hoveredView(NULL),
  _pickImage(0), _splitter(NULL), _dragSplitter(false), _fontSize(16.f), 
  _name(name), _forceRedraw(0), _idle(false), _popup(NULL), _fbo(0),  _tex(0)
{
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  glfwWindowHint(GLFW_RED_BITS,mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS,mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS,mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE,mode->refreshRate);
  
  //glfwWindowHint(GLFW_DECORATED, false);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_SAMPLES, 4);

  _window = glfwCreateWindow(mode->width, mode->height, "Jivaro.1.0",  monitor, NULL);
  if (!_window) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    _window = glfwCreateWindow(mode->width, mode->height, "Jivaro.1.0", monitor, NULL);
  }
  if (!_window) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    _window = glfwCreateWindow(mode->width, mode->height, "Jivaro.1.0", monitor, NULL);
    LEGACY_OPENGL = true;
  }
  BuildKeyMap();
  _width = mode->width;
  _height = mode->height;
  _shared = true;

}

// width/height window constructor
//----------------------------------------------------------------------------
Window::Window(int width, int height, const std::string& name):
  _pixels(NULL), _debounce(0),_mainView(NULL), _activeView(NULL), _hoveredView(NULL),
  _pickImage(0), _splitter(NULL), _dragSplitter(false), _fontSize(16.f), 
  _name(name), _forceRedraw(0), _idle(false), _popup(NULL), _fbo(0), _tex(0)
{
  _width = width;
  _height = height;
  _shared = true;
  //glfwWindowHint(GLFW_DECORATED, false);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_SAMPLES, 4);
  
  _window = glfwCreateWindow(_width,_height,"Jivaro",NULL,NULL);
  if (!_window) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    _window = glfwCreateWindow(_width, _height, "Jivaro", NULL, NULL);
  }
  if (!_window) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    _window = glfwCreateWindow(_width, _height, "Jivaro", NULL, NULL);
    LEGACY_OPENGL = true;
  }
  BuildKeyMap();
}

// child window constructor
//----------------------------------------------------------------------------
Window::Window(int x, int y, int width, int height, 
  GLFWwindow* parent, const std::string& name, bool decorated) :
  _pixels(NULL), _debounce(0), _mainView(NULL), _activeView(NULL), _hoveredView(NULL),
  _pickImage(0), _splitter(NULL), _dragSplitter(false), _fontSize(16.f), 
  _name(name), _forceRedraw(0), _idle(false), _popup(NULL), _fbo(0), _tex(0)
{
  _width = width;
  _height = height;
  _shared = false;

  glfwWindowHint(GLFW_DECORATED, decorated);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_FLOATING, true);
  glfwWindowHint(GLFW_SAMPLES, 4);

  _window = glfwCreateWindow(_width, _height, name.c_str(), NULL, parent);
  if (!_window) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    _window = glfwCreateWindow(_width, _height, name.c_str(), NULL, parent);
  }
  if (!_window) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    _window = glfwCreateWindow(_width, _height, name.c_str(), NULL, parent);
    LEGACY_OPENGL = true;
  }
  glfwSetWindowPos(_window, x, y);
  BuildKeyMap();
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
    //glfwSwapInterval(0);

    if (_shared) {
      GetContextVersionInfos();
      // load opengl functions
      GarchGLApiLoad();
      pxr::GlfContextCaps::InitInstance();
      pxr::GlfContextCaps const& caps = pxr::GlfContextCaps::GetInstance();
      CreateFontAtlas();
      InitializeIcons();
      InitializeTools();
    }

    // setup callbacks
    glfwSetWindowSizeCallback(_window, ResizeCallback);
    glfwSetMouseButtonCallback(_window, ClickCallback);
    glfwSetScrollCallback(_window, ScrollCallback);
    glfwSetKeyCallback(_window, KeyboardCallback);
    glfwSetCharCallback(_window, CharCallback);
    glfwSetCursorPosCallback(_window, MouseMoveCallback);
    glfwSetWindowFocusCallback(_window, FocusCallback);
    glfwSetInputMode(_window, GLFW_STICKY_KEYS, GLFW_TRUE);

    // create main splittable view
    _mainView = new View(NULL, pxr::GfVec2f(0,0), pxr::GfVec2f(_width, _height));
    _mainView->SetWindow(this);
    _splitter = new SplitterUI();

    Resize(_width, _height);
    glGenVertexArrays(1, &_vao);
    
    // ui
    SetupImgui();
    glfwMakeContextCurrent(NULL);
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

  if(_shared) {
    DeleteFontAtlas();
  }
}

// create full screen window
//----------------------------------------------------------------------------
Window* 
Window::CreateFullScreenWindow()
{
  return new Window(true, "Jivaro");
}

// create standard window
//----------------------------------------------------------------------------
Window*
Window::CreateStandardWindow(int width, int height)
{
  return new Window(width, height, "Jivaro");
}

// child window
//----------------------------------------------------------------------------
Window*
Window::CreateChildWindow(
  int x, int y, int width, int height, GLFWwindow* parent,
  const std::string& name, bool decorated)
{
  return new Window(x, y, width, height, parent, name, decorated);
}

// force redraw
//----------------------------------------------------------------------------
void
Window::ForceRedraw()
{
  for (View* leaf : _leaves)leaf->SetFlag(View::FORCEREDRAW);
  _forceRedraw = 1;
}

// popup
//----------------------------------------------------------------------------
void
Window::SetPopup(PopupUI* popup)
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
  glBlitFramebuffer(0, 0, _width, _height, 0, _height, _width, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  _popup = popup;
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void
Window::UpdatePopup(PopupUI* popup)
{
  if (popup) {
    if (!popup->IsDone())return;
    delete popup;
  }
  SetPopup(NULL);
  ForceRedraw();
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

  if (_fbo) glDeleteFramebuffers(1, &_fbo);
  if (_tex) glDeleteTextures(1, &_tex);

  // setup background buffer for popup window
  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  glGenTextures(1, &_tex);
  glBindTexture(GL_TEXTURE_2D, _tex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tex, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void
Window::SetActiveView(View* view)
{
  if (_activeView)
  {
    if (_activeView == view)return;
    _activeView->ClearFlag(View::ACTIVE);
    _activeView->ClearFlag(View::INTERACTING);
    _activeView->SetDirty();
  }
  if(view)
  {
    _activeView = view;
    _activeView->SetFlag(View::ACTIVE);
    _activeView->SetDirty();
  }
  else _activeView = NULL;
}

void
Window::SetHoveredView(View* view)
{
  if (_hoveredView)
  {
    if (_hoveredView == view)return;
    _hoveredView->ClearFlag(View::OVER);
    _hoveredView->SetDirty();
  }
  if (view)
  {
    _hoveredView = view;
    _hoveredView->SetFlag(View::OVER);
    _hoveredView->SetDirty();
  }
  else _hoveredView = NULL;
}

// split view
//----------------------------------------------------------------------------
View* 
Window::SplitView(View* view, double perc, bool horizontal, int fixed, int numPixels)
{
  if(!view->GetFlag(View::LEAF))
  {
    return NULL;
  }
  
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
  
  view->SetPerc(perc);
  view->ClearFlag(View::LEAF);
  CollectLeaves(_mainView);
  BuildSplittersMap();
  return view;
}

// collect leaves views (contains actual ui elements)
//----------------------------------------------------------------------------
void 
Window::CollectLeaves(View* view)
{
  if(view == NULL || view == _mainView) {
    view = _mainView;
    _leaves.clear();
  }
  if (view->GetFlag(View::LEAF)) {
    _leaves.push_back(view);
    view->SetDirty();
  } else {
    if(view->GetLeft())CollectLeaves(view->GetLeft());
    if(view->GetRight())CollectLeaves(view->GetRight());
  }
}

// get view (leaf) under mouse
//----------------------------------------------------------------------------
View*
Window::GetViewUnderMouse(int x, int y)
{
  for(auto leaf: _leaves) {
    if (leaf->Contains(x, y)) {
      leaf->SetDirty();
      return leaf;
    }
  }
  return NULL;
}

void 
Window::DirtyViewsUnderBox(const pxr::GfVec2i& min, const pxr::GfVec2i& size)
{
  for (auto leaf : _leaves) {
    if (leaf->Intersect(min, size)) {
      leaf->SetDirty();
    }
  }
}

void
Window::DiscardMouseEventsUnderBox(const pxr::GfVec2i& min, const pxr::GfVec2i& size)
{
  for (auto leaf : _leaves) {
    if (leaf->Intersect(min, size)) {
      leaf->SetFlag(View::DISCARDMOUSEBUTTON | View::DISCARDMOUSEMOVE);
    }
  }
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
  // get GLFW, OpenGL version:
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

static ImGuiWindowFlags JVR_BACKGROUND_FLAGS =
  ImGuiWindowFlags_NoInputs |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoDecoration |
  ImGuiWindowFlags_NoBackground;


// draw
//----------------------------------------------------------------------------
void 
Window::Draw()
{
  if (!_valid || _idle)return;
  SetGLContext();
  glBindVertexArray(_vao);
  // start the imgui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  
  // draw popup
  if (_popup) {
    
    if (! _popup->IsSync()) {
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImVec2(_width, _height));
      ImGui::Begin("##background", NULL, JVR_BACKGROUND_FLAGS);

      ImDrawList* drawList = ImGui::GetBackgroundDrawList();
      drawList->AddImage(ImTextureID(_tex), ImVec2(0, 0), ImVec2(_width, _height),
        ImVec2(0, 0), ImVec2(1, 1), ImColor(100,100,100, 255));
      ImGui::End();
    } else {
      for (Engine* engine : GetApplication()->GetEngines()) {
        engine->SetDirty(true);
      }
      _mainView->Draw(true);
    }
    _popup->Draw();
    if (_popup->IsDone() || _popup->IsCancel()) {
      delete _popup;
      _popup = NULL;
    }
  } else {
    // draw views
    if (_mainView)_mainView->Draw(_forceRedraw > 0);
    _forceRedraw = pxr::GfMax(0, _forceRedraw - 1);

    // draw splitters
    _splitter->Draw();
  }

  // render the imgui frame
  ImGui::Render();
  
  glViewport(0, 0, (int)_io->DisplaySize.x, (int)_io->DisplaySize.y);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glBindVertexArray(0);
}

// setup imgui
//----------------------------------------------------------------------------
void 
Window::SetupImgui()
{
  static float fontSizes[3] = { 16.f,32.f,64.f };
  // setup imgui context
  IMGUI_CHECKVERSION();
  _context = ImGui::CreateContext(SHARED_ATLAS);
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
  SetStyle(&ImGui::GetStyle());

  // setup platform/renderer bindings
  pxr::GlfContextCaps const& caps = pxr::GlfContextCaps::GetInstance();
  ImGui_ImplGlfw_InitForOpenGL(_window, false);
  if (caps.glVersion < 330) {
    ImGui_ImplOpenGL3_Init("#version 120 ");
  }
  else {
    ImGui_ImplOpenGL3_Init("#version 330 ");
  }
}

// clear imgui
//----------------------------------------------------------------------------
void 
Window::ClearImgui()
{
  ImGui::DestroyContext(_context);
  // Cleanup
  if(_shared) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
  }
}

void Window::DragSplitter(int x, int y)
{
  if(_activeView) {
    _activeView->GetPercFromMousePosition(x, y);
    _mainView->Resize(0, 0, _width, _height, false);
    _splitter->Resize(_width, _height);
    _splitter->RecurseBuildMap(_mainView);
  }
}

bool Window::UpdateActiveTool(int x, int y)
{
  if(GetActiveView()) {
    GetActiveView()->MouseMove(x, y);
  }
  return false;
}

void Window::MainLoop()
{
  glfwSwapInterval(1);
  while(!glfwWindowShouldClose(_window)) {
    //glfwWaitEventsTimeout(1.f / (60 * APPLICATION->GetTime().GetFPS()));
    glfwPollEvents();
    //glfwWaitEvents();
    _app->Update();
    // main window
    Draw();
    glfwSwapBuffers(_window);
    
    // child windows
    for (auto& child : _childrens) {
      if (glfwGetWindowAttrib(child->GetGlfwWindow(), GLFW_FOCUSED)) {
        child->Draw();
        glfwSwapBuffers(child->GetGlfwWindow());
      }
    }
  }
}

// pick splitter
//----------------------------------------------------------------------------
bool 
Window::PickSplitter(double mouseX, double mouseY)
{
  int splitterIndex = _splitter->Pick(mouseX, mouseY);
  if(splitterIndex >= 0) {
    View* activeView = _splitter->GetViewByIndex(splitterIndex);
    if (activeView->GetFlag(View::HORIZONTAL)) {
      _splitter->SetVerticalCursor();
    } else {
      _splitter->SetHorizontalCursor();
    }
    return true;
  } else {
    _splitter->SetDefaultCursor();
  }
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
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
  Window* parent = (Window*)glfwGetWindowUserPointer(window);
  PopupUI* popup = parent->GetPopup();
  if (popup) {
    popup->Keyboard(key, scancode, action, mods);
    parent->UpdatePopup(popup);
    return;
  }
  Application* app = parent->GetApplication();
  Time& time = app->GetTime();
  
  if(action == GLFW_RELEASE) {
    parent->SetDebounce(false);
  }
  if (action == GLFW_PRESS) {
    switch(GetMappedKey(key))
    {
      case GLFW_KEY_Z:
      {
        if (mods & GLFW_MOD_CONTROL)app->Undo();
        break;
      }
      case GLFW_KEY_Y: 
      {
        if (mods & GLFW_MOD_CONTROL)app->Redo();
        break;
      }
      case GLFW_KEY_D:
      {
        if (mods & GLFW_MOD_CONTROL)app->Duplicate();
        break;
      }
      case GLFW_KEY_DELETE:
      {
        app->Delete();
        break;
      }
      case GLFW_KEY_SPACE:
      {
        if (time.IsPlaying()) {
          time.SetLoop(false);
          time.StopPlayBack();
        }
        else {
          time.SetLoop(true);
          time.StartPlayBack();
        }
        break;
      }
      case GLFW_KEY_LEFT:
      {
        time.PreviousFrame();
        break;
      }

      case GLFW_KEY_RIGHT:
      {
        time.NextFrame();
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
        if (view) {
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
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  bool splitterHovered = parent->PickSplitter(x, y);

  PopupUI* popup = parent->GetPopup();
  if (popup) {
    popup->MouseButton(button, action, mods);
    parent->UpdatePopup(popup);
  } else if(button == GLFW_MOUSE_BUTTON_RIGHT && mods == 0) {
    View* view = parent->GetActiveView();
    if (view) {
      view->MouseButton(button, action, mods);
    }
  } else {
    if (action == GLFW_RELEASE)
    {
      //parent->SetActiveTool(TOOL_SELECT);
      View* view = parent->GetActiveView();
      if (view) {
        view->MouseButton(button, action, mods);
        view->ClearFlag(View::INTERACTING);
        view->SetDirty();
      }
      if (parent->IsDraggingSplitter()) {
        parent->EndDragSplitter();
        parent->Resize(width, height);
      }
    }
    else if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      if (splitterHovered) {
        View* activeView = parent->GetActiveView();
        if (activeView) { activeView->SetInteracting(false); };
        parent->SetActiveView(parent->GetSplitter()->GetHovered());
        parent->BeginDragSplitter();
      }
      else {
        View* view = parent->GetViewUnderMouse((int)x, (int)y);
        if (view) {
          parent->SetActiveView(view);
          view->SetFlag(View::INTERACTING);
          view->MouseButton(button, action, mods);
        }
      }
    }
  }
}

void 
ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  Window* parent = Window::GetUserData(window);
  PopupUI* popup = parent->GetPopup();
  ImGui::SetCurrentContext(parent->GetContext());
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
  if (popup) {
    popup->MouseWheel(xoffset, yoffset);
  } else if(parent->GetActiveView()) {
    parent->GetActiveView()->MouseWheel(xoffset, yoffset);
  }
}

void 
CharCallback(GLFWwindow* window, unsigned c)
{
  Window* parent = Window::GetUserData(window);
  PopupUI* popup = parent->GetPopup();
  ImGui_ImplGlfw_CharCallback(window, c);
  if (popup) {
    popup->Input(c);
  } else if (parent->GetActiveView()) {
    parent->GetActiveView()->Input(c);
  }
}

void 
MouseMoveCallback(GLFWwindow* window, double x, double y)
{
  Window* parent = Window::GetUserData(window);
  PopupUI* popup = parent->GetPopup();
  ImGui::SetCurrentContext(parent->GetContext());
  View* view = parent->GetViewUnderMouse((int)x, (int)y);
  View* active = parent->GetActiveView();
  bool splitterHovered = parent->PickSplitter(x, y);

  if (popup) {
    popup->MouseMove(x, y);
  } else if(parent->IsDraggingSplitter()) {
    parent->DragSplitter(x, y);
  } else {
    if (active && active->GetFlag(View::INTERACTING)) {
      active->MouseMove(x, y);
    } else {
      if (view) {
        parent->SetHoveredView(view);
        parent->GetHoveredView()->MouseMove(x, y);
      }
    }
  }
}

void FocusCallback(GLFWwindow* window, int focused)
{
  if (focused) {
    Window* parent = Window::GetUserData(window);
    GetApplication()->SetActiveWindow(parent);
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

JVR_NAMESPACE_CLOSE_SCOPE