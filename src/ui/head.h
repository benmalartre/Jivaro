#ifndef JVR_UI_HEAD_H
#define JVR_UI_HEAD_H

#include <iostream>
#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../ui/utils.h"
#include "../ui/ui.h"


PXR_NAMESPACE_OPEN_SCOPE

#define VIEW_HEAD_HEIGHT 14

class View;

class ViewHead
{
public:
  static ImGuiWindowFlags _flags;
  ViewHead(View* parent);
  ~ViewHead();

  void CreateChild(UIType type);
  void AddChild(BaseUI* child);
  void RemoveChild(BaseUI* child);
  void SetCurrentChild(int index);

  // overrides
  //void MouseButton(int action, int button, int mods) override {};
  //void MouseMove(int x, int y) override {};
  void Draw();
  void MouseMove(int x, int y);
  void MouseButton(int button, int action, int mods);

  /*
  MenuItem& AddItem(View* view, const std::string label, const std::string shortcut, bool selected,
    bool enabled, MenuPressedFunc f = NULL, const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());*/
private:
  //std::vector<HeadItem>   _items;
  //HeadItem*               _current;
  int                     _current;
  View*                   _parent;
  std::vector<BaseUI*>    _childrens;
  bool                    _invade;
  std::string             _name;

};

class HeadedUI : public BaseUI
{
public:
  HeadedUI(View* parent, const std::string& name);
  ~HeadedUI();

  int GetX() override;
  int GetY() override;
  int GetWidth() override;
  int GetHeight() override;
  void GetRelativeMousePosition(const float inX, const float inY,
    float& outX, float& outY) override;

private:
  ViewHead*                 _head;
  static ImGuiWindowFlags   _flags;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_HEAD_H