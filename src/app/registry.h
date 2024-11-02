#ifndef JVR_WINDOW_REGISTRY_H
#define JVR_WINDOW_REGISTRY_H

#include <vector>
#include <functional>

#include "../app/window.h"
#include "../app/view.h"
#include "../ui/ui.h"


JVR_NAMESPACE_OPEN_SCOPE

class WindowRegistry {
public:
  // constructor
  WindowRegistry() : _mainWindow(nullptr), _activeWindow(nullptr), _focusWindow(nullptr){};
  
  // singleton
  static WindowRegistry* Get();

  Window* GetMainWindow() {return _mainWindow;};
  Window* GetChildWindow(size_t index) {return _childWindows[index];};
  std::vector<Window*>& GetChildWindows(){return _childWindows;};
  const std::vector<Window*>& GetChildWindows() const {return _childWindows;};
  Window* GetActiveWindow() { return _activeWindow ? _activeWindow : _mainWindow; };
  void SetActiveWindow(Window* window) { _activeWindow = window; };
  void SetFocusWindow(Window* window) { _focusWindow = window; };
  void AddWindow(Window* window);
  void RemoveWindow(Window* window);
  void SetWindowDirty(Window* window);
  void SetAllWindowsDirty();

  void Update();

  void SetActiveTool(size_t t);
  bool IsToolInteracting();

  // create a fullscreen window
  Window* CreateFullScreenWindow(const std::string& name);

  // create a standard window of specified size
  Window* CreateStandardWindow(const std::string& name, const GfVec4i& dimension);

  // create a child window
  Window* CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent);

private:
  static WindowRegistry*            _singleton;

  Window*                           _mainWindow;
  std::vector<Window*>              _childWindows;
  Window*                           _activeWindow;
  Window*                           _focusWindow;

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