#ifndef JVR_UI_EXPLORER_H
#define JVR_UI_EXPLORER_H

#include <limits>
#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/hashmap.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

class Selection;
class ExplorerUI : public BaseUI
{
public:
  struct Item {
    SdfPath    path;
    size_t          id;
    bool            selected;
  };

  ExplorerUI(View* parent);
  ~ExplorerUI() override;

  void MouseButton(int action, int button, int mods) override;
  void MouseMove(int x, int y) override;
  void Keyboard(int key, int scancode, int action, int mods) override;
  void Init();
  bool Draw() override;
  void OnSelectionChangedNotice(const SelectionChangedNotice& n) override;

protected:
  void _DrawItemBackground(ImDrawList* drawList, bool selected, bool& flip);
  void _DrawBackground();
  void _DrawPrim(const UsdPrim& prim, Selection* selection);
  void _DrawVisibility(const UsdPrim& prim, bool visible, bool selected);
  void _DrawActive(const UsdPrim& prim, bool active, bool selected);
  void _DrawType(const UsdPrim& prim, bool selected);
  Item* _GetItemUnderMouse(const GfVec2f& relative);

private:
  bool                          _locked;
  bool                          _flip;
  SdfPath                       _current;
  size_t                        _selectionHash;
  std::vector<Item>             _items;
  size_t                        _mapping;
  bool                          _drag;
  std::vector<SdfPath>          _dragItems;
  GfVec2f                       _scroll;

  static ImGuiWindowFlags       _flags;
  static ImGuiTreeNodeFlags     _treeFlags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_EXPLORER_H