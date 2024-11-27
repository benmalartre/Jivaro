#ifndef JVR_WINDOW_REGISTRY_H
#define JVR_WINDOW_REGISTRY_H

#include <vector>
#include <map>
#include <functional>

#include <pxr/base/gf/vec4i.h>
#include <pxr/usd/usd/stage.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Window;
class View;
class BaseUI;
class PopupUI;
class Engine;

class EngineRegistry {
public:
  // engines
  static void AddEngine(Engine* engine);
  static void RemoveEngine(Engine* engine);
  static void SetActiveEngine(Engine* engine);
  static Engine* GetActiveEngine();
  static std::vector<Engine*> GetEngines();

  // singleton
  static EngineRegistry* Get();
  static void UpdateAllEnginesSelection(const SdfPathVector& selected);

private:
  // engines
  std::vector<Engine*>              _engines;
  Engine*                           _activeEngine;
};

class WindowRegistry {
public:
  // preferences
  static bool         PlaybackAllViews;

  // constructor
  WindowRegistry() : _activeWindow(nullptr), _popup(nullptr), _playbackView(nullptr){};
  
  // singleton
  static WindowRegistry* Get();

  static Window* GetWindow(size_t index);
  static std::vector<Window*>& GetWindows();
  static Window* GetActiveWindow();
  static void SetActiveWindow(Window* window);
  static void AddWindow(Window* window);
  static void RemoveWindow(Window* window);
  static void SetWindowDirty(Window* window);
  static void SetAllWindowsDirty();

  // playback viewport
  static bool IsPlaybackView(View* view);
  static void SetPlaybackView(View* view);

  static void ApplyPendingModifications();
  static bool Update();

  static void SetActiveTool(size_t t);
  static bool IsToolInteracting();

  // popup
  static PopupUI* GetPopup();
  static void SetPopup(PopupUI* popup);
  static void UpdatePopup();

  // create a fullscreen window
  static Window* CreateFullScreenWindow(const std::string& name);

  // create a standard window of specified size
  static Window* CreateStandardWindow(const std::string& name, const GfVec4i& dimension);

  // create a child window
  static Window* CreateChildWindow(const std::string& name, const GfVec4i& dimension, Window* parent);

private:
  std::vector<Window*>              _windows;
  Window*                           _activeWindow;
  View*                             _playbackView;

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
    _creators[identifier] =  Wrapper<ConcreteUI>();
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