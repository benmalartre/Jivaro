#ifndef JVR_UI_HEAD_H
#define JVR_UI_HEAD_H

#include <iostream>
#include <sstream>  
#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../common.h"
#include "../ui/utils.h"
#include "../ui/ui.h"


JVR_NAMESPACE_OPEN_SCOPE

#define VIEW_HEAD_HEIGHT 32
const ImVec2 JVR_MINI_BUTTON_SIZE(14, 14);
const char* VIEW_HEAD_NAME = "view_head_";

static size_t ViewHeadID = 0;

class ViewHead
{
public:
  static ImGuiWindowFlags _flags;
  ViewHead(View* parent);
  ~ViewHead();

  void CreateChild(UIType type);
  void AddChild(BaseUI* child);
  void RemoveChild(int index);
  void SetCurrentChild(int index);
  BaseUI* GetCurrentChild();
  void SetView(View* view);

  float GetHeight() { return _height;};
  const std::vector<BaseUI*>& GetChildrens() const { return _childrens; };

  // overrides
  bool Draw();
  void MouseMove(int x, int y);
  void MouseButton(int button, int action, int mods);
  bool OnButtonClicked(int btn);

private:
  static const char*      _ComputeName(int index, const char* suffix="");
  int                     _current;
  View*                   _parent;
  std::vector<BaseUI*>    _childrens;
  bool                    _invade;
  int                     _id;
  float                   _height;
  const char*             _name;
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