#include "../utils/glutils.h"
#include "../utils/files.h"
#include "../utils/icons.h"
#include "../utils/keys.h"
#include "../ui/style.h"
#include "../ui/utils.h"
#include "../ui/viewport.h"
#include "../ui/splitter.h"
#include "../ui/popup.h"
#include "../app/application.h"
#include "../app/tools.h"
#include <chrono>
#include <thread>
#include <pxr/imaging/glf/contextCaps.h>
#include <pxr/base/arch/systemInfo.h>

#include "../app/window.h"
#include "../app/view.h"
#include "../geometry/shape.h"


#include "../utils/files.h"
#include "../utils/timer.h"
#include "../utils/prefs.h"
#include "../ui/fileBrowser.h"
#include "../ui/viewport.h"
#include "../ui/menu.h"
#include "../ui/popup.h"
#include "../ui/graphEditor.h"
#include "../ui/timeline.h"
#include "../ui/demo.h"
#include "../ui/toolbar.h"
#include "../ui/explorer.h"
#include "../ui/layers.h"
#include "../ui/layerEditor.h"
#include "../ui/propertyEditor.h"
#include "../ui/curveEditor.h"

JVR_NAMESPACE_OPEN_SCOPE

bool LEGACY_OPENGL = false;

int MAPPED_KEYS[GLFW_KEY_LAST + 1];
bool KEY_MAP_INITIALIZED = false;
float REPEAT_KEY_DURATION = 0.1f * 1e9;

static ImGuiWindowFlags JVR_BACKGROUND_FLAGS =
  ImGuiWindowFlags_NoInputs |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoDecoration |
  ImGuiWindowFlags_NoBackground;


// width/height window constructor
//----------------------------------------------------------------------------
Window::Window(const std::string& name, const pxr::GfVec4i& dimension, bool fullscreen, Window* parent) :
  _pixels(NULL), _debounce(0),_mainView(NULL), _activeView(NULL), _hoveredView(NULL),
  _splitter(NULL), _dragSplitter(false), _fontSize(16.f), _name(name), _forceRedraw(3), _idle(false), 
  _fbo(0), _tex(0), _layout(std::numeric_limits<int>::max()), _needUpdateLayout(true)
{
  GLFWmonitor* monitor = NULL;
  if (fullscreen) {
    monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    _width = mode->width;
    _height = mode->height;
  }
  else {
    _width = dimension[2];
    _height = dimension[3];
  }

  glfwWindowHint(GLFW_DECORATED, fullscreen - 1);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_DOUBLEBUFFER, true);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_SAMPLES, 4);

  _shared = !(bool)parent;
  _window = glfwCreateWindow(_width, _height, name.c_str(), monitor, parent ? parent->GetGlfwWindow() : NULL);
  
  if(_window) {
    if(parent)glfwSetWindowPos(_window, dimension[0], dimension[1]);
    Init();
  }
}

// initialize
//----------------------------------------------------------------------------
void 
Window::Init()
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
  }

  // setup callbacks
  glfwSetWindowSizeCallback(_window, WindowSizeCallback);
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
  _splitter = new SplitterUI(this);
    
  Resize(_width, _height);

  glGenVertexArrays(1, &_vao);

  GLSLProgram* pgm = InitShapeShader((void*)this);
  _tool.SetProgram(pgm);
    
  // ui
  SetupImgui();
  glfwMakeContextCurrent(NULL);
}

// window destructor
//----------------------------------------------------------------------------
Window::~Window()
{
  if (_fbo) glDeleteFramebuffers(1, &_fbo);
  if (_tex) glDeleteTextures(1, &_tex);
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
Window::CreateFullScreenWindow(const std::string& name)
{
  return new Window("Jivaro", pxr::GfVec4i(), true);
}

// create standard window
//----------------------------------------------------------------------------
Window*
Window::CreateStandardWindow(const std::string& name, const pxr::GfVec4i& dimension)
{
  return new Window(name, dimension, false);
}

// child window
//----------------------------------------------------------------------------
Window*
Window::CreateChildWindow(const std::string& name, const pxr::GfVec4i& dimension, Window* parent)
{
  return new Window(name, dimension, false, parent);
}

// layouts
//----------------------------------------------------------------------------
static void _BaseLayout(Window* window)
{
  window->ClearViews();
  View* mainView = window->GetMainView();

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);

  window->SplitView(mainView, 0.5, true, View::LFIXED, 32);
  View* menuView = mainView->GetLeft();
  menuView->SetTabed(false);

  window->Resize(width, height);
  new MenuUI(menuView);
}

static void _RandomLayout(Window* window)
{
  window->ClearViews();
  View* mainView = window->GetMainView();

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);

  window->SplitView(mainView, 0.5, true, View::LFIXED, 32);
  View* menuView = mainView->GetLeft();
  menuView->SetTabed(false);

  window->Resize(width, height);
  new MenuUI(menuView);
}

static void _StandardLayout(Window* window)
{
  window->ClearViews();
  View* mainView = window->GetMainView();

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);
  window->SplitView(mainView, 0.5, true, View::LFIXED, 22);

  View* bottomView = mainView->GetRight();
  window->SplitView(bottomView, 0.9, true, false);

  View* timelineView = bottomView->GetRight();
  timelineView->SetTabed(false);

  View* centralView = bottomView->GetLeft();
  window->SplitView(centralView, 0.6, true);

  View* middleView = centralView->GetLeft();
  View* menuView = mainView->GetLeft();
  menuView->SetTabed(false);

  window->SplitView(middleView, 0.8, false);

  View* workingView = middleView->GetLeft();
  window->SplitView(workingView, 0.25, false);

  View* propertyView = middleView->GetRight();
  View* leftTopView = workingView->GetLeft();
  window->SplitView(leftTopView, 0.1, false, View::LFIXED, 32);

  View* toolView = leftTopView->GetLeft();
  toolView->SetTabed(false);
  View* explorerView = leftTopView->GetRight();
  View* viewportView = workingView->GetRight();
  View* graphView = centralView->GetRight();

  window->Resize(width, height);

  uint64_t Ts[8];
  
  Ts[0] = CurrentTime();
  ViewportUI* viewport = new ViewportUI(viewportView);
  Ts[1] = CurrentTime();
  new TimelineUI(timelineView);
  Ts[2] = CurrentTime();
  new MenuUI(menuView);
  Ts[3] = CurrentTime();
  new ToolbarUI(toolView, true);
  Ts[4] = CurrentTime();
  new ExplorerUI(explorerView);
  Ts[5] = CurrentTime();
  new PropertyEditorUI(propertyView);
  Ts[6] = CurrentTime();
  //new GraphEditorUI(graphView);
  Ts[7] = CurrentTime();
  GetApplication()->SetActiveEngine(viewport->GetEngine());

  std::string names[7] = { "viewport", "timeline", "menu", "toolbar", "explorer", "property", "graph" };

  for (size_t t = 0; t < 7; ++t) {
    std::cout << names[t] << " : " << (double)((Ts[t + 1] - Ts[t]) * 1e-9) << " seconds" << std::endl;
  }
}

static void _RawLayout(Window* window)
{
  window->SetGLContext();
  window->ClearViews();
  View* mainView = window->GetMainView();

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);

  window->SplitView(mainView, 0.5, true, View::LFIXED, 24);
  View* menuView = mainView->GetLeft();
  menuView->SetTabed(false);

  View* middleView = mainView->GetRight();
  window->SplitView(middleView, 0.5, true, View::RFIXED, 64);

  View* viewportView = middleView->GetLeft();
  View* timelineView = middleView->GetRight();
  timelineView->SetTabed(false);

  window->Resize(width, height);

  new MenuUI(menuView);
  new ViewportUI(viewportView);
  new TimelineUI(timelineView);
}

// force redraw
//----------------------------------------------------------------------------
void
Window::ForceRedraw()
{
  for (View* leaf : _leaves)leaf->SetFlag(View::FORCEREDRAW);
  _forceRedraw++;
}

void
Window::ClearViews()
{
  _activeView = _hoveredView = _activeView = _mainView;
  _mainView->Clear();
  _leaves.clear();
  _uic.clear();
}

// ui names
//----------------------------------------------------------------------------
std::string
Window::ComputeUniqueUIName(short type)
{
  std::string baseName = UITypeName[type];
  if (_uic.find(baseName) != _uic.end()) {
    _uic[baseName]++;
  }
  else {
    _uic[baseName] = 1;
  }
  return baseName + std::to_string(_uic[baseName]);
}

// popup
//----------------------------------------------------------------------------
void
Window::CaptureFramebuffer()
{
  SetGLContext();
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
  glBlitFramebuffer(0, 0, _width, _height, 0, _height, _width, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


// repeat key
//----------------------------------------------------------------------------
void 
Window::BeginRepeatKey()
{
  _lastRepeatT = CurrentTime();
}

void 
Window::EndRepeatKey()
{
  _lastRepeatT = std::numeric_limits<uint64_t>::max();
}

bool 
Window::ShouldRepeatKey()
{
  uint64_t currentT = CurrentTime();
  if ((currentT - _lastRepeatT) > REPEAT_KEY_DURATION) {
    _lastRepeatT = currentT;
    return true;
  }
  return false;
}



// Resize
//----------------------------------------------------------------------------
void 
Window::Resize(unsigned width, unsigned height)
{
  const bool resolutionChanged = (_width != width) || (_height != height);
  _width = width;
  _height = height;

  CollectLeaves();
  _mainView->Resize(0, 0, _width, _height, true);
  _splitter->Resize(_width, _height);

  if(_width <= 0 || _height <= 0)_valid = false;
  else _valid = true;

  if (resolutionChanged) {
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
}

// Layout
//------------------------------------------------------------------------------
void
Window::SetLayout()
{
  if (_needUpdateLayout) {
    switch (_layout) {
    case 0:
      _BaseLayout(this);
      break;
    case 1:
      _RawLayout(this);
      break;
    case 2:
      _StandardLayout(this);
      break;
    case 3:
    default:
      _RandomLayout(this);
      break;
    }
    _needUpdateLayout = false;
  }
}

size_t 
Window::GetLayout() {
  return _layout;
}

void 
Window::SetDesiredLayout(size_t layout)
{
  if (layout != _layout) {
    _layout = layout;
    _needUpdateLayout = true;
  } else {
    _needUpdateLayout = false;
  }
}

void
Window::SetActiveView(View* view)
{
  if (_activeView)
  {
    if (_activeView == view)return;
    _activeView->Focus(false);
    _activeView->ClearFlag(View::ACTIVE);
    _activeView->ClearFlag(View::INTERACTING);
    _activeView->SetDirty();
  }
  if(view)
  {
    _activeView = view;
    _activeView->Focus(true);
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
  Resize(_width, _height);
  return view;
}

// remove view
//----------------------------------------------------------------------------
void 
Window::RemoveView(View* view)
{
  if (view == _mainView) {
    std::cerr << "[view] can't remove main view !" << std::endl;
    return;
  }
  if(_activeView == view)_activeView = NULL;
  if(_hoveredView == view)_hoveredView = NULL;
  View* parent = view->GetParent();
  View* sibling = NULL;
  if(parent->GetLeft() == view) {
    sibling = parent->GetRight();
  } else if(parent->GetRight() == view) {
    sibling = parent->GetLeft();
  }
  std::cout << "remove view " << view << std::endl;
  std::cout << "sibling view " << sibling << std::endl;

  if (sibling) {
    if (sibling->GetFlag(View::LEAF)) {
      parent->TransferUIs(sibling);
      parent->SetFlag(View::LEAF);
      parent->SetPerc(1.f);
    }
    else {     
      parent->SetLeft(sibling->GetLeft());
      parent->SetRight(sibling->GetRight());
      parent->SetPerc(sibling->GetPerc());
      parent->SetFlags(sibling->GetFlags());
      sibling->SetLeft(NULL);
      sibling->SetRight(NULL);
    }
    std::cout << "delete sibbling" << std::endl;
    delete sibling;
  }

  std::cout << "delete view" << std::endl;
  delete view;
  Resize(_width, _height);
  ForceRedraw();
  _mainView->SetDirty();
  std::cout << "remove view done" << std::endl;
}

// collect child leaves views from specified view
//----------------------------------------------------------------------------
void _CollectLeaves(std::vector<View*>& views, std::vector<View*>& leaves, View* view)
{
  views.push_back(view);
  if (view->GetFlag(View::LEAF)) {
    leaves.push_back(view);
  } else {
    if (view->GetLeft())_CollectLeaves(views, leaves, view->GetLeft());
    if (view->GetRight())_CollectLeaves(views, leaves, view->GetRight());
  }
}

void 
Window::CollectLeaves()
{
  _leaves.clear();
  _views.clear();
  _CollectLeaves(_views, _leaves, _mainView);
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
Window::DirtyViewsUnderBox(const pxr::GfVec2f& min, const pxr::GfVec2f& size)
{
  for (auto leaf : _leaves) {
    if (leaf->Intersect(min, size)) {
      leaf->SetDirty();
    }
  }
}

void
Window::DiscardMouseEventsUnderBox(const pxr::GfVec2f& min, const pxr::GfVec2f& size)
{
  for (auto leaf : _leaves) {
    if (leaf->Intersect(min, size)) {
      leaf->SetFlag(View::DISCARDMOUSEBUTTON | View::DISCARDMOUSEMOVE);
    }
  }
}

void
Window::InvalidateViews()
{
  _activeView = _hoveredView = _activeLeaf = NULL;
}

const std::vector<View*>&
Window::GetLeaves()
{
  return _leaves;
}

const std::vector<View*>&
Window::GetViews()
{
  return _views;
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

// draw
//----------------------------------------------------------------------------
void 
Window::Draw()
{
  if (!_valid || _idle)return;

  SetGLContext();
  SetLayout();

  glBindVertexArray(_vao);
  
  // start the imgui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // draw views
  if (_mainView)_mainView->Draw(_forceRedraw > 0);
  _forceRedraw = pxr::GfMax(0, _forceRedraw - 1);

  // draw splitters
  _splitter->Draw();

  // render the imgui frame
  ImGui::Render();

  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glBindVertexArray(0);

  //glFlush();
  glfwSwapBuffers(_window);
  glFinish();
}

// draw
//----------------------------------------------------------------------------
void
Window::DrawPopup(PopupUI* popup)
{
  if (!_valid || _idle)return;
  SetGLContext();
  glBindVertexArray(_vao);

  // start the imgui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();


  if (!popup->IsSync()) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(GetWidth(), GetHeight()));
    ImGui::Begin("##background", NULL, JVR_BACKGROUND_FLAGS);
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    drawList->AddImage((ImTextureID)(uintptr_t)_tex, ImVec2(0, 0), ImVec2(_width, _height),
      ImVec2(0, 0), ImVec2(1, 1), ImColor(100, 100, 100, 255));
    ImGui::End();
  } else {
    for (Engine* engine : GetApplication()->GetEngines()) {
      engine->SetHighlightSelection(false);
      engine->SetDirty(true);
    }
    GetMainView()->Draw(true);
  }

  popup->Draw();
  
  // render the imgui frame
  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glBindVertexArray(0);

  //glFlush();
  glfwSwapBuffers(_window);
  glFinish();
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
  _io->FontAllowUserScaling = true;
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
  }
}

bool Window::UpdateActiveTool(int x, int y)
{
  if(GetActiveView()) {
    GetActiveView()->MouseMove(x, y);
  }
  return false;
}

bool Window::Update()
{
  if (IsIdle())return true;
  if (glfwWindowShouldClose(_window)) {
    if (!_shared) {
      GetApplication()->RemoveWindow(this);
      delete this;
    }
    return false;
  }
  SetGLContext();
  Draw();
  return true;
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
  Application* app = GetApplication();
  PopupUI* popup = app->GetPopup();
  if (popup) {
    popup->Keyboard(key, scancode, action, mods);
    app->UpdatePopup();
    return;
  }

  Time& time = app->GetTime();
  
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
          parent->BeginRepeatKey();
        }
      }
    }
  } else if (action == GLFW_REPEAT) {
    View* view = parent->GetActiveView();
    if (view && parent->ShouldRepeatKey()) {
      view->Keyboard(key, scancode, action, mods);
    }
  } else if (action == GLFW_RELEASE) {
    parent->SetDebounce(false);
    parent->EndRepeatKey();
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
  Application* app = GetApplication();
  app->SetActiveWindow(parent);
  ImGui::SetCurrentContext(parent->GetContext());
  

  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  bool splitterHovered = parent->PickSplitter(x, y);

  PopupUI* popup = app->GetPopup();
  if (popup) {
    popup->MouseButton(button, action, mods);
    app->UpdatePopup();
  } else if(button == GLFW_MOUSE_BUTTON_RIGHT && mods == 0) {
    View* view = parent->GetActiveView();
    if (view) {
      view->MouseButton(button, action, mods);
    }
  } else {
    if (action == GLFW_RELEASE)
    {
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
    } else {
      if (splitterHovered) {
        View* activeView = parent->GetActiveView();
        if (activeView) { activeView->SetInteracting(false); };
        parent->SetActiveView(parent->GetSplitter()->GetHovered());
        parent->BeginDragSplitter();
      } else {
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
  PopupUI* popup = GetApplication()->GetPopup();
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
  PopupUI* popup = GetApplication()->GetPopup();
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
  PopupUI* popup = GetApplication()->GetPopup();
  ImGui::SetCurrentContext(parent->GetContext());
  View* hovered = parent->GetViewUnderMouse((int)x, (int)y);
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
      if (hovered) {
        parent->SetHoveredView(hovered);
        hovered->MouseMove(x, y);
      }
    }
  }
}

void 
FocusCallback(GLFWwindow* window, int focused)
{
  if (focused) {
    Window* parent = Window::GetUserData(window);
    GetApplication()->SetFocusWindow(parent);
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
WindowSizeCallback(GLFWwindow* window, int width, int height)
{  
  Window* parent = (Window*)glfwGetWindowUserPointer(window);
  glViewport(0, 0, width, height);
  parent->SetGLContext();
  parent->Resize(width, height);
  parent->Draw();
}

static void _RecurseSplitRandomLayout(View* view, size_t depth, size_t maxDepth)
{
  if (depth > maxDepth)return;
  view->Split(RANDOM_0_1, (rand() % 100) > 50);
  _RecurseSplitRandomLayout(view->GetLeft(), depth + 1, maxDepth);
  _RecurseSplitRandomLayout(view->GetRight(), depth + 1, maxDepth);
}

JVR_NAMESPACE_CLOSE_SCOPE