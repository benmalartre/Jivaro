#ifndef JVR_UI_EXPLORER_H
#define JVR_UI_EXPLORER_H

#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/hashmap.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"


PXR_NAMESPACE_OPEN_SCOPE

extern IconList ICONS;
static size_t EXPLORER_LINE_HEIGHT = 20;

struct ExplorerItem {
  pxr::UsdPrim                  _prim;
  bool                          _visible;
  bool                          _selected;
  bool                          _expanded;
  std::vector<ExplorerItem*>    _items;

  ExplorerItem() 
    : _prim(pxr::UsdPrim())
    , _visible(true)
    , _selected(false)
    , _expanded(true) {};

  ExplorerItem(pxr::UsdPrim& prim, bool visible, bool selected, bool expanded)
    : _prim(prim)
    , _visible(visible)
    , _selected(selected)
    , _expanded(expanded) {};

  ExplorerItem* AddItem(pxr::UsdPrim& prim, bool visible, bool selected, bool expanded) {
    _items.push_back(new ExplorerItem(prim, visible, selected, expanded));
    return _items.back();
  }
  ~ExplorerItem() {
    for (auto item : _items)delete item;
  };
};

typedef pxr::TfHashMap<pxr::SdfPath, ExplorerItem*, pxr::TfHash> ExplorerItemMap;

class ExplorerUI : public BaseUI
{
public:
  ExplorerUI(View* parent);
  ~ExplorerUI()         override;

  void MouseButton(int action, int button, int mods) override;
  void MouseMove(int x, int y) override;
  void Keyboard(int key, int scancode, int action, int mods) override;
  void Init();
  void Update();
  void Select();
  bool Draw()      override;

  void RecurseStage();
  void RecursePrim(ExplorerItem* current);
  void DrawItem(ExplorerItem* item, bool heritedVisibility);
  void DrawItemType(ExplorerItem* item);
  void DrawItemVisibility(ExplorerItem* item, bool heritedVisibility);
  void DrawBackground(float localMouseX, float localMouseY);
  void DrawItemBackground(ImDrawList* drawList, const ExplorerItem* item, 
    bool& flip);

  void OnSceneChangedNotice(const SceneChangedNotice& n) override;
  void OnSelectionChangedNotice(const SelectionChangedNotice& n) override;

private:
  void _UpdateSelection(ExplorerItem* item, bool isLeaf);

  pxr::GfVec3f                  _color;
  bool                          _locked;
  ExplorerItem*                 _last;
  ExplorerItem*                 _current;
  ExplorerItem*                 _root;
  ExplorerItemMap               _map;
  size_t                        _selectionHash;

  static ImGuiWindowFlags       _flags;
  static ImGuiTreeNodeFlags     _treeFlags;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_EXPLORER_H