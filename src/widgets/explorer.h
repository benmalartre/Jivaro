#ifndef AMN_WIDGETS_EXPLORER_H
#define AMN_WIDGETS_EXPLORER_H
#pragma once

#include "../common.h"
#include "../app/ui.h"
#include "../utils/ui.h"
#include "../utils/utils.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

extern AmnIconMap AMN_ICONS;
static size_t AMN_EXPLORER_LINE_HEIGHT = 20;

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

class ExplorerUI : public BaseUI
{
public:
  ExplorerUI(View* parent);
  ~ExplorerUI()         override;

  void MouseButton(int action, int button, int mods) override {};
  void MouseMove(int x, int y) override {};
  void Init();
  void Update();
  bool Draw()      override;

  void RecurseStage();
  void RecursePrim(ExplorerItem* current);
  void DrawItem(ExplorerItem* item, bool heritedVisibility);
  void DrawItemType(ExplorerItem* item);
  void DrawItemVisibility(ExplorerItem* item, bool heritedVisibility);
  void DrawBackground();
  void DrawItemBackground(ImDrawList* drawList, const ExplorerItem* item, bool& flip);

private:
  pxr::GfVec3f                  _color;
  bool                          _locked;
  ExplorerItem*                 _root;
  ImGuiTreeNodeFlags            _selectBaseFlags;
  Icon*                         _visibleIcon;
  Icon*                         _invisibleIcon;
  ImVec4                        _backgroundColor;
  ImVec4                        _alternateColor;
  ImVec4                        _selectedColor;
  ImVec4                        _hoveredColor;
  bool                          _needRefresh;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_WIDGETS_EXPLORER_H