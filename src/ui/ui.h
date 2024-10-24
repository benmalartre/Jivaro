#ifndef JVR_UI_UI_H
#define JVR_UI_UI_H

#include <map>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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
  MAINMENU,
  TIMELINE,
  TOOLBAR,
  FILEBROWSER,
  POPUP,
  SPLITTER,
  VIEWPORT,
  EXPLORER,
  PROPERTYEDITOR,
  CURVEEDITOR,
  GRAPHEDITOR,
  LAYEREDITOR,
  TEXTEDITOR,
  CONTENTBROWSER,
  DEBUG,
  DEMO,
  ICON,
  TOOL,
  COUNT
};

static const char* UITypeName[UIType::COUNT] = {
  "mainmenu",
  "timeline",
  "toolbar",
  "fileBrowser",
  "popup",
  "splitter",
  "viewport",
  "explorer",
  "propertyEditor",
  "curveEditor",
  "graphEditor", 
  "layerEditor",
  "textEditor",
  "contentBrowser",
  "debug",
  "demo",
  "icon",
  "tool"
};

typedef std::map<std::string, int> UITypeCounter;


class HeadUI;

struct ViewEventData {
  enum Type {
    NONE,
    MOUSE_BUTTON,
    MOUSE_MOVE,
    KEYBOARD_INPUT
  };

  short type;
  int button;
  int action;
  int mods;
  int x;
  int y;
  int key;
  int width;
  int height;
};


class BaseUI : public TfWeakBase
{
public:
  BaseUI(View* parent, short type, bool popup=false);
  virtual ~BaseUI(){};

  // get ui type
  short GetType() { return _type; };

  // get parent window
  virtual Window* GetWindow();

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
  virtual GfVec2f GetPosition();

  // get the (x,y) size in window space (left top corner)
  virtual GfVec2f GetSize();

  // get the x position in window space (x-coordinate of left top corner)
  virtual int GetX();

  // get the y position in window space (y-coordinate of left top corner)
  virtual int GetY();

  // get the width of the parent view
  virtual int GetWidth();
  
  // get the height of the parent view
  virtual int GetHeight();

  // discard events if mouse cursor inside relative bbox
  void DiscardEventsIfMouseInsideBox(const GfVec2f& min, const GfVec2f& max);

  // attach tooltip
  void AttachTooltip(const char* tooltip);

  // get unique name
  std::string ComputeUniqueName(short type);

  const std::string& GetName() const {return _name;};

  virtual void MouseButton(int button, int action, int mods){};
  virtual void MouseMove(int x, int y){}; 
  virtual void MouseWheel(int x, int y){};
  virtual void Keyboard(int key, int scancode, int action, int mods) {};
  virtual void Input(int key) {};

  void SetInteracting(bool state);
  void SetParent(View* view) { _parent = view; };
  virtual bool Draw()=0;
  virtual void Resize(){};

  inline bool IsInteracting(){return _interacting;};

  // notices callbacks
  virtual void OnNewSceneNotice(const NewSceneNotice& n);
  virtual void OnSceneChangedNotice(const SceneChangedNotice& n);
  virtual void OnSelectionChangedNotice(const SelectionChangedNotice& n);
  virtual void OnAttributeChangedNotice(const AttributeChangedNotice& n);
  virtual void OnAllNotices(const TfNotice& n);
  
protected:
  // execution event
  ViewEventData _MouseButtonEventData(int button, int action, int mods, int x, int y);
  ViewEventData _MouseMoveEventData(int x, int y);

  short                   _type;
  bool                    _initialized;
  bool                    _interacting;
  View*                   _parent;
  std::string             _name;
  static ImGuiWindowFlags _flags;
  static bool             _headed;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif