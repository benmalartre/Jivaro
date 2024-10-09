#ifndef JVR_UI_MENU_H
#define JVR_UI_MENU_H

#include <vector>
#include <functional>

#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include "../pbd/menu.h"

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
    std::vector<Item*>          items;

    Item(MenuUI* ui, Item* parent, const std::string& label, bool selected, bool enabled, CALLBACK_FN cb = NULL);
    ~Item();
    Item* Add(const std::string& label, bool selected, bool enabled, CALLBACK_FN cb = NULL);

    void Draw(bool* modified, size_t itemIdx);
    friend MenuUI;
  };
  
public:
  MenuUI(View* parent);
  ~MenuUI();

  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
  void DirtyViewsBehind();

  Item* Add(const std::string& label, bool selected, bool enabled, CALLBACK_FN cb = NULL);

private:
  pxr::GfVec2f            _ComputeSize(const Item* item);
  pxr::GfVec2f            _ComputePos(const Item* item);
  std::vector<Item*>      _items;
  std::vector<size_t>     _opened;
  static ImGuiWindowFlags _flags;
};

static void OpenFileCallback();
static void SaveFileCallback();
static void NewFileCallback();
static void OpenDemoCallback();
static void OpenChildWindowCallback();
static void SetLayoutCallback(Window* window, short layout);
static void CreatePrimCallback(short type);
static void TriangulateCallback();
static void FlattenGeometryCallback();

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_MENU_H