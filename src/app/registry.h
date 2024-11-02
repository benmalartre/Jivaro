#ifndef JVR_WINDOW_REGISTRY_H
#define JVR_WINDOW_REGISTRY_H

#include <vector>
#include <map>
#include <functional>

#include <pxr/base/gf/vec4i.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Window;
class View;
class BaseUI;
class PopupUI;

class WindowRegistry {
public:
  
  // constructor
  WindowRegistry() : _activeWindow(nullptr), _popup(nullptr){};
  
  // singleton
  static WindowRegistry* New();
  static WindowRegistry* Get();

  Window* GetWindow(size_t index) {return _windows[index];};
  std::vector<Window*>& GetWindows(){return _windows;};
  const std::vector<Window*>& GetWindows() const {return _windows;};
  Window* GetActiveWindow() { return _activeWindow;};
  void SetActiveWindow(Window* window) { _activeWindow = window; };
  void AddWindow(Window* window);
  void RemoveWindow(Window* window);
  void SetWindowDirty(Window* window);
  void SetAllWindowsDirty();

  bool Update();

  void SetActiveTool(size_t t);
  bool IsToolInteracting();

  // popup
  PopupUI* GetPopup() { return _popup; };
  void SetPopup(PopupUI* popup);
  void UpdatePopup();

  // create a fullscreen window
  Window* CreateFullScreenWindow(const std::string& name);

  // create a standard window of specified size
  Window* CreateStandardWindow(const std::string& name, const GfVec4i& dimension);

  // create a child window
  Window* CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent);

private:
  std::vector<Window*>              _windows;
  Window*                           _activeWindow;

  // uis
  PopupUI*                          _popup;

};


class UIRegistry
{
  typedef std::function<BaseUI*(View*)> Creator;
  typedef std::map<std::string, Creator> Creators;

public:

  // singleton
  static UIRegistry* Get();

  template <class ConcreteUI>
  void Register(const std::string &identifier)
  {
    _creators[identifier] =  Wrapper<ConcreteUI>;
  }

  BaseUI *Create(View* view, const std::string &identifier)
  {
    Creators::iterator it = _creators.find(identifier);
    if (it != _creators.end())
    {
      return it->second(view);
    }
    return nullptr;
  }

protected:

  template<class ConcreteUI>
  struct Wrapper
  {
    BaseUI *operator()(View* view) const { return new ConcreteUI(view); }
  };

private:
  Creators            _creators;
  static UIRegistry*  _singleton;

};


JVR_NAMESPACE_CLOSE_SCOPE

#endif