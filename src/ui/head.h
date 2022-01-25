#ifndef JVR_UI_HEAD_H
#define JVR_UI_HEAD_H

#include <iostream>
#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../ui/utils.h"
#include "../ui/ui.h"


PXR_NAMESPACE_OPEN_SCOPE

#define JVR_HEAD_HEIGHT 24

// callback prototype
typedef void(*MenuPressedFunc)(const pxr::VtArray<pxr::VtValue>& args);

/*
struct HeadItem {
  View* view;
  std::string                 label;

  std::vector<MenuItem>       items;
  pxr::VtArray<pxr::VtValue>  args;
  MenuPressedFunc             func;

  HeadItem(View* view, const std::string lbl, const std::string sht, bool sel,
    bool enb, MenuPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());

  bool Draw();
};
*/
class View;
class ViewHead;

class HeadedUI : public BaseUI
{
public:
  HeadedUI(View* parent, const std::string& name);
  ~HeadedUI();

  // overrides
  //void MouseButton(int action, int button, int mods) override {};
  //void MouseMove(int x, int y) override {};

  int GetX() override;
  int GetY() override;
  int GetWidth() override;
  int GetHeight() override;

  /*
  MenuItem& AddItem(View* view, const std::string label, const std::string shortcut, bool selected,
    bool enabled, MenuPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());*/
private:
  //std::vector<HeadItem>   _items;
  //HeadItem*               _current;
  ViewHead*                 _head;
  static ImGuiWindowFlags _flags;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_HEAD_H