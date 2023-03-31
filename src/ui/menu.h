#ifndef JVR_UI_MENU_H
#define JVR_UI_MENU_H

#include <iostream>
#include <vector>
#include <functional>
#include <tuple>
#include <memory>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>

#include "../ui/ui.h"
#include "../ui/utils.h"


#include <thread>
JVR_NAMESPACE_OPEN_SCOPE

class Command;


typedef void(*Callback)(void*);


class MenuUI : public BaseUI
{
public:

  struct Item {
    MenuUI*                     ui;
    Item*                       parent;
    std::vector<Item>           items;
    std::string                 label;
    bool                        selected;
    bool                        enabled;
    Callback                    callback;
    void*                       args;

    Item(MenuUI* ui, const std::string label, bool selected, bool enabled, Callback cb, void* args);

    Item& Add(const std::string label, bool selected, bool enabled, Callback cb=NULL, void* args=NULL);

    bool Draw();
    pxr::GfVec2i ComputeSize();
    pxr::GfVec2i ComputePos();

  };

public:
  MenuUI(View* parent);
  ~MenuUI();

  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
  void DirtyViewsUnderBox();

  Item& Add(const std::string label, bool selected, bool enabled, Callback cb = NULL, void* args = NULL);

private:
  std::vector<Item>       _items;
  Item*                   _current;
  static ImGuiWindowFlags _flags;
  pxr::GfVec2i            _pos;
  pxr::GfVec2i            _size;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_MENU_H