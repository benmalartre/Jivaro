#ifndef JVR_UI_HEAD_H
#define JVR_UI_HEAD_H

#include <iostream>
#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../common.h"
#include "../ui/utils.h"
#include "../ui/ui.h"


JVR_NAMESPACE_OPEN_SCOPE

#define VIEW_HEAD_HEIGHT 32

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

  float GetHeight() { return _height;};

  // overrides
  //void MouseButton(int action, int button, int mods) override {};
  //void MouseMove(int x, int y) override {};
  bool Draw();
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
  float                   _height;
};

class HeadedUI : public BaseUI
{
public:
  HeadedUI(View* parent, short type);
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

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_HEAD_H