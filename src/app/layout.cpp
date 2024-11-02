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
#include "../ui/tool.h"
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

  int width, height;
  glfwGetWindowSize(window->GetGlfwWindow(), &width, &height);
  window->SplitView(mainView, 0.5, true, View::LFIXED, window->GetMenuBarHeight());

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
  new ToolUI(graphView);
  //AttributeEditorUIditorUI(graphView);
  Ts[7] = CurrentTime();
  Application::Get()->GetModel()->SetActiveEngine(viewport->GetEngine());

  std::string names[7] = { "viewport", "timeline", "menu", "toolbar", "explorer", "property", "graph" };

  for (size_t t = 0; t < 7; ++t) {
    std::cout << names[t] << " : " << (double)((Ts[t + 1] - Ts[t]) * 1e-9) << " seconds" << std::endl;
  }
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