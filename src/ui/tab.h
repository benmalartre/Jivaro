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

#define VIEW_TAB_HEIGHT 32
static const char* VIEW_TAB_NAME = "view_tab_";

class ViewTabUI
{
  static int ViewTabUIID;
public:
  static ImGuiWindowFlags _flags;
  ViewTabUI(View* parent);
  ~ViewTabUI();

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

private:
  static std::string      _ComputeName(int index, const char* suffix="");
  int                     _current;
  View*                   _parent;
  std::vector<BaseUI*>    _childrens;
  bool                    _invade;
  int                     _id;
  float                   _height;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_HEAD_H