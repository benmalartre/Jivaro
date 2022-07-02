#ifndef JVR_UI_LAYER_H
#define JVR_UI_LAYER_H

#include <pxr/usd/usd/prim.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

extern IconList ICONS;
static size_t EXPLORER_LINE_HEIGHT = 20;

struct ExplorerItem {
  pxr::UsdPrim                  _prim;
  bool                          _visible;
  bool                          _selected;
  bool                          _expanded;
  std::vector<ExplorerItem*>    _items;

  ExplorerItem* AddItem() {
    _items.push_back(new ExplorerItem());
    return _items.back();
  }
  ~ExplorerItem() {
    for (auto item : _items)delete item;
  };
};

class LayerUI : public BaseUI
{
public:
  LayerUI(View* parent);
  ~LayerUI()         override;

  void MouseButton(int action, int button, int mods) override;
  void MouseMove(int x, int y) override;
  void Init();
  void Update();
  bool Draw()      override;

  void RecurseStage();
  void RecursePrim(ExplorerItem* current);
  void DrawItem(ExplorerItem* item, bool heritedVisibility);
  void DrawItemType(ExplorerItem* item);
  void DrawItemVisibility(ExplorerItem* item, bool heritedVisibility);
  void DrawBackground(float localMouseX, float localMouseY);
  void DrawItemBackground(ImDrawList* drawList, const ExplorerItem* item, 
    bool& flip);

private:
  void _UpdateSelection(ExplorerItem* item, bool isLeaf);

  pxr::GfVec3f                  _color;
  bool                          _locked;
  ExplorerItem*                 _root;
  Icon*                         _visibleIcon;
  Icon*                         _invisibleIcon;
  bool                          _needRefresh;

  static ImGuiWindowFlags       _flags;
  static ImGuiTreeNodeFlags     _treeFlags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_EXPLORER_H