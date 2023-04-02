#ifndef JVR_UI_EXPLORER_H
#define JVR_UI_EXPLORER_H

#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/hashmap.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/tab.h"
#include "../ui/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

static size_t EXPLORER_LINE_HEIGHT = 20;
class Selection;
class ExplorerUI : public BaseUI
{
  struct Item {
    pxr::SdfPath    path;
    size_t          id;
    bool            selected;
  };

public:
  ExplorerUI(View* parent);
  ~ExplorerUI() override;

  //void MouseButton(int action, int button, int mods) override;
  //void MouseMove(int x, int y) override;
  //void Keyboard(int key, int scancode, int action, int mods) override;
  //void Init();
  //void Update();
  //void Select();
  bool Draw() override;

  void DrawItemBackground(ImDrawList* drawList, bool selected, bool& flip);
  void DrawBackground();
  void DrawPrim(const pxr::UsdPrim& prim, Selection* selection);
  void DrawVisibility(const pxr::UsdPrim& prim, bool visible, bool selected);
  void DrawType(const pxr::UsdPrim& prim, bool selected);
  void DrawActive(const pxr::UsdPrim& prim, bool selected);

private:
  //void _UpdateSelection(Item* item, bool isLeaf);

  bool                          _locked;
  bool                          _flip;
  pxr::SdfPath                  _current;
  size_t                        _selectionHash;
  std::vector<Item>             _items;
  size_t                        _mapping;

  static ImGuiWindowFlags       _flags;
  static ImGuiTreeNodeFlags     _treeFlags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_EXPLORER_H