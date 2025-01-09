#include "../utils/timer.h"
#include "../app/layout.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/time.h"
#include "../app/application.h"

#include "../ui/fileBrowser.h"
#include "../ui/viewport.h"
#include "../ui/menu.h"
#include "../ui/popup.h"
#include "../ui/graphEditor.h"
#include "../ui/timeline.h"
#include "../ui/demo.h"
#include "../ui/toolbar.h"
#include "../ui/commands.h"
#include "../ui/explorer.h"
#include "../ui/propertyEditor.h"


JVR_NAMESPACE_OPEN_SCOPE

// layouts
//----------------------------------------------------------------------------
void Layout::BaseLayout(Window* window)
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

void Layout::RandomLayout(Window* window)
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

void Layout::StandardLayout(Window* window)
{
  window->SetGLContext();
  window->ClearViews();
  View* mainView = window->GetMainView();
  Index* index = window->GetIndex();

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);
  window->SplitView(mainView, 0.5, true, View::LFIXED, window->GetMenuBarHeight());

  View* menuView = mainView->GetLeft();
  menuView->SetTabed(false);

  View* view = mainView->GetRight();
  window->SplitView(view, 0.9, true, false);

  View* timelineView = view->GetRight();
  timelineView->SetTabed(false);

  view = view->GetLeft();
  window->SplitView(view, 0.6, true, false);

  View* graphView = view->GetRight();
  View* centralView = view->GetLeft();
  
  window->SplitView(centralView, 0.1, false, View::LFIXED, 32);

  View* toolView = centralView->GetLeft();
  toolView->SetTabed(false);

  View* middleView = centralView->GetRight();

  window->SplitView(middleView, 0.8, false);

  View* workingView = middleView->GetLeft();
  View* propertyView = middleView->GetRight();

  window->SplitView(workingView, 0.25, false);

  View* explorerView = workingView->GetLeft();
  View* viewportView = workingView->GetRight();
  
  window->Resize(width, height);

  uint64_t Ts[8];
  
  ViewportUI* viewport = new ViewportUI(viewportView);
  new TimelineUI(timelineView);
  new MenuUI(menuView);
  new ToolbarUI(toolView, true);
  new ExplorerUI(explorerView);
  new PropertyEditorUI(propertyView);
  new CommandsUI(graphView);
  //AttributeEditorUIditorUI(graphView);
  EngineRegistry::SetActiveEngine(viewport->GetEngine());
}

void Layout::RawLayout(Window* window)
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


JVR_NAMESPACE_CLOSE_SCOPE