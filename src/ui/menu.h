#ifndef JVR_UI_MENU_H
#define JVR_UI_MENU_H

#include <vector>
#include <functional>

#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../ui/ui.h"
#include "../ui/utils.h"

JVR_NAMESPACE_OPEN_SCOPE

class Command;

class MenuUI : public BaseUI
{
public:
  struct Item {
    MenuUI*                     ui;
    Item*                       parent;
    std::vector<Item>           items;
    std::string                 label;
    bool                        selected;
    bool                        enabled;
    UIUtils::CALLBACK_FN        callback;

    Item(MenuUI* ui, const std::string label, bool selected, bool enabled, UIUtils::CALLBACK_FN cb=NULL);
    Item& Add(const std::string label, bool selected, bool enabled, UIUtils::CALLBACK_FN cb=NULL);

    bool Draw();
    pxr::GfVec2i ComputeSize();
    pxr::GfVec2i ComputePos();
  };

public:
  MenuUI(View* parent);
  ~MenuUI();

  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
  void DirtyViewsUnderBox();

  Item& Add(const std::string label, bool selected, bool enabled, UIUtils::CALLBACK_FN cb=NULL);

private:
  std::vector<Item>       _items;
  Item*                   _current;
  static ImGuiWindowFlags _flags;
  pxr::GfVec2i            _pos;
  pxr::GfVec2i            _size;
};


static void OpenFileCallback();
static void SaveFileCallback();
static void NewFileCallback();
static void OpenDemoCallback();
static void OpenChildWindowCallback();
static void SetLayoutCallback(Window* window, short layout);
static void CreatePrimCallback();
static void TriangulateCallback();
static void FlattenGeometryCallback();

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_MENU_H