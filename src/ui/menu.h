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

/*
template <typename Func, typename A, typename ...Args> 
struct Caller
{
  static void _Call(Func& f, A&& a, Args && ...args)
  {
    f(std::forward<A>(a));
    Caller<Func, Args...>::_Call(f, std::forward<Args>(args)...);
  }
};

template <typename Func, typename A> 
struct Caller<Func, A>
{
  static void _Call(Func& f, A&& a)
  {
    f(std::forward<A>(a));
  }
};

template <typename Func, typename ...Args>
void Callback(Func& f, Args && ...args)
{
  Caller<Func, Args...>::_Call(f, std::forward<Args>(args)...);
}

*/

typedef void(*Callback)(...);


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
    pxr::VtArray<pxr::VtValue>  args;
    std::function<void()>       func;

    Item(MenuUI* ui, const std::string label, bool selected, bool enabled, 
      Callback cb=NULL, const pxr::VtArray<pxr::VtValue>& args= pxr::VtArray<pxr::VtValue>());

    Item& Add(const std::string label, bool selected, bool enabled, Callback cb = NULL);
    template<typename... Args>
    Item& Add(const std::string label, bool selected, bool enabled, Callback cb, Args... args);

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

  Item& Add(const std::string label, bool selected, bool enabled, 
    Callback cb=NULL, const pxr::VtArray<pxr::VtValue>& args= pxr::VtArray<pxr::VtValue>());

private:
  std::vector<Item>       _items;
  Item*                   _current;
  static ImGuiWindowFlags _flags;
  pxr::GfVec2i            _pos;
  pxr::GfVec2i            _size;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_MENU_H