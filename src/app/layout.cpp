#include "../app/layout.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../ui/contentBrowser.h"
#include "../ui/menu.h"
#include "../ui/explorer.h"
#include "../ui/fileBrowser.h"
#include "../ui/graphEditor.h"
#include "../ui/layerEditor.h"
#include "../ui/propertyEditor.h"
#include "../ui/timeline.h"
#include "../ui/toolbar.h"

JVR_NAMESPACE_OPEN_SCOPE

Layout::Layout()
  : _view(NULL), _splitter(NULL), _width(0), _height(0)
{
}

Layout::~Layout()
{
  if (_splitter)delete _splitter;
  if (_view)delete _view;
}

void Layout::Set(Window* window, short layout)
{
  switch (layout) {
  case 0:
    _StandardLayout(window);
    break;
  case 1:
    _RawLayout(window);
    break;
  default:
    _StandardLayout(window);
    break;
  }
}

void Layout::SetDirty()
{
  _view->SetDirty();
}

void Layout::Draw(bool forceRedraw)
{
  _view->Draw(forceRedraw);
  _splitter->Draw();
}

void Layout::Resize(size_t width, size_t height)
{
  _view->Resize(0, 0, width, height, false);
  _splitter->Resize(width, height);
}

void Layout::_Reset(Window* window)
{
  glfwGetWindowSize(window->GetGlfwWindow(), &_width, &_height);
  if (_view) delete _view;
  if (_splitter) delete _splitter;

  _view = new View(NULL, pxr::GfVec2f(0, 0), pxr::GfVec2f(_width, _height));
  _view->SetWindow(window);
  _splitter = new SplitterUI(_view);
}


void Layout::_StandardLayout(Window* window) 
{
  _Reset(window);
  View* mainView = window->SplitView(
    window->GetMainView(), 0.5, true, View::LFIXED, 22);
  View* bottomView = window->SplitView(
    mainView->GetRight(), 0.9, true, false);
  //bottomView->Split(0.9, true, true);
  View* timelineView = bottomView->GetRight();
  timelineView->SetTabed(false);
  View* centralView = window->SplitView(
    bottomView->GetLeft(), 0.6, true);
  View* middleView = centralView->GetLeft();
  View* topView = mainView->GetLeft();
  topView->SetTabed(false);

  window->SplitView(middleView, 0.75, false);

  View* workingView = window->SplitView(
    middleView->GetLeft(), 0.25, false);
  View* propertyView = middleView->GetRight();
  View* leftTopView = window->SplitView(
    workingView->GetLeft(), 0.5, false, View::LFIXED, 32);
  View* toolView = leftTopView->GetLeft();
  toolView->SetTabed(false);
  View* explorerView = leftTopView->GetRight();
  /*
  _mainWindow->SplitView(stageView, 0.25, true);
  View* layersView = stageView->GetLeft();
  View* explorerView = stageView->GetRight();
  */

  View* viewportView = workingView->GetRight();
  View* graphView = centralView->GetRight();

  window->Resize(_width, _height);

  new GraphEditorUI(graphView);

  //CurveEditorUI* editor = new CurveEditorUI(graphView);
  new ViewportUI(viewportView);
  new TimelineUI(timelineView);
  new MenuUI(topView);
  new ToolbarUI(toolView, true);
  new ExplorerUI(explorerView);
  new PropertyUI(propertyView);

  //_layers =  new LayersUI(layersView);
  //new LayerHierarchyUI(layersView, "fuck");
  //_property = new PropertyUI(propertyView, "Property");
  //new DemoUI(propertyView);
}

void Layout::_RawLayout(Window* window) 
{
  if(window->GetMainView()) {
    delete window->GetMainView();
  }
  View* mainView = window->SplitView(
    window->GetMainView(), 0.5, true, View::LFIXED, 100);

  View* viewportView = mainView->GetRight();
  View* graphView = mainView->GetLeft();
}

JVR_NAMESPACE_CLOSE_SCOPE