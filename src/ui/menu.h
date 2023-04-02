#ifndef JVR_UI_MENU_H
#define JVR_UI_MENU_H

#include <vector>
#include <functional>

#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../common.h"
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
    std::string                 label;
    bool                        selected;
    bool                        enabled;
    CALLBACK_FN                 callback;
    std::vector<Item>           items;

    Item* Add(const std::string label, bool selected, bool enabled, CALLBACK_FN cb = NULL);
  };

public:
  MenuUI(View* parent);
  ~MenuUI();

  pxr::GfVec2i ComputeSize(const Item& item);
  pxr::GfVec2i ComputePos(const Item& item);

  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
  void DirtyViewsUnderBox();

  Item* Add(const std::string label, bool selected, bool enabled, CALLBACK_FN cb = NULL);

private:
  bool                    _Draw(const Item& item);
  std::vector<Item>       _items;
  static ImGuiWindowFlags _flags;
  pxr::GfVec2i            _pos;
  pxr::GfVec2i            _size;
};


static void OpenFileCallback();
static void SaveFileCallback();
static void NewFileCallback();
static void OpenDemoCallback();
static void OpenChildWindowCallback();
static void CreatePrimCallback();
static void TriangulateCallback();
static void FlattenGeometryCallback();
static void SetLayoutCallback(Window*, size_t);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_MENU_H