#ifndef JVR_UI_UI_H
#define JVR_UI_UI_H

#include <map>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../common.h"
#include "../app/notice.h"

#include <pxr/base/tf/notice.h>
#include <pxr/base/tf/weakBase.h>
#include <pxr/base/tf/instantiateType.h>
#include <pxr/base/gf/vec2f.h>

JVR_NAMESPACE_OPEN_SCOPE  

#define UI_HEADER_HEIGHT 32

class View;
class Window;
class Application;

enum UIType {
  TIMELINE,
  TOOLBAR,
  FILEBROWSER,
  // headed uis
  VIEWPORT,
  EXPLORER,
  PROPERTY,
  CURVEEDITOR,
  GRAPHEDITOR,
  DEBUG,
  DEMO,
  COUNT
};

static const char* UITypeName[UIType::COUNT] = {
  "timeline",
  "toolbar",
  "fileBrowser",
  "viewport",
  "explorer",
  "property",
  "curveEditor",
  "graphEditor", 
  "debug",
  "demo"
};

static std::map<std::string, int> UINameIndexMap;


class HeadUI;
class BaseUI : public pxr::TfWeakBase
{
public:
  BaseUI(View* parent, const std::string& name, bool popup=false);
  virtual ~BaseUI(){};

  // get parent window
  Window* GetWindow();

  // get the parent view
  View* GetView() { return _parent; };

  // get parent window height
  int GetWindowHeight();

  // mouse position in the view space
  // (0, 0) left top corner
  // (width, height) right bottom corner
  virtual void GetRelativeMousePosition(const float inX, const float inY, 
    float& outX, float& outY);

  // get the (x,y) position in window space (left top corner)
  virtual pxr::GfVec2f GetPosition();

  // get the x position in window space (x-coordinate of left top corner)
  virtual int GetX();

  // get the y position in window space (y-coordinate of left top corner)
  virtual int GetY();

  // get the width of the parent view
  virtual int GetWidth();
  
  // get the height of the parent view
  virtual int GetHeight();

  // discard events if mouse cursor inside relative bbox
  void DiscardEventsIfMouseInsideBox(const pxr::GfVec2f& min, const pxr::GfVec2f& max);

  // attach tooltip
  void AttachTooltip(const char* tooltip);

  // get unique name
  std::string ComputeUniqueName(const std::string& baseName);

  const std::string& GetName() const {return _name;};

  virtual void MouseButton(int button, int action, int mods){};
  virtual void MouseMove(int x, int y){}; 
  virtual void MouseWheel(int x, int y){};
  virtual void Keyboard(int key, int scancode, int action, int mods) {};
  virtual void Input(int key) {};

  void SetInteracting(bool state);
  virtual bool Draw()=0;
  virtual void Resize(){};

  // notices callbacks
  virtual void OnNewSceneNotice(const NewSceneNotice& n);
  virtual void OnSceneChangedNotice(const SceneChangedNotice& n);
  virtual void OnSelectionChangedNotice(const SelectionChangedNotice& n);
  virtual void OnAllNotices(const pxr::TfNotice& n);
  
protected:
  bool                    _initialized;
  bool                    _interacting;
  View*                   _parent;
  std::string             _name;
  static ImGuiWindowFlags _flags;
  static bool             _headed;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif