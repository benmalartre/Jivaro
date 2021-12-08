#ifndef AMN_UI_TOOLBAR_H
#define AMN_UI_TOOLBAR_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"

#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>


AMN_NAMESPACE_OPEN_SCOPE

enum ToolbarItemType {
  TOOLBAR_BUTTON,
  TOOLBAR_SEPARATOR
};

// callback prototype
//typedef void(*ToolbarPressedFunc)(const pxr::VtArray<pxr::VtValue>& args);
struct ToolbarItem {
  BaseUI*                     ui;
  ToolbarItemType             type;

  ToolbarItem(BaseUI* ui, ToolbarItemType type) 
    : ui(ui), type(type) {};
  virtual ~ToolbarItem(){};
  virtual bool Draw()=0;
};

enum ToolbarSeparatorOrientation : short {
  SEPARATOR_VERTICAL,
  SEPARATOR_HORIZONTAL
};

struct ToolbarSeparator : public ToolbarItem {
  short orientation;

  ToolbarSeparator(BaseUI* ui, 
    short orientation=ToolbarSeparatorOrientation::SEPARATOR_VERTICAL);
  ~ToolbarSeparator(){};
  bool Draw() override;
};

struct ToolbarButton : public ToolbarItem {
  short                       tool;
  std::string                 label;
  std::string                 shortcut;
  std::string                 tooltip;
  bool                        toggable;
  bool                        enabled;

  pxr::VtArray<pxr::VtValue>  args;
  IconPressedFunc             func;
  Icon*                       icon;

  ToolbarButton(BaseUI* ui, short tool, const std::string& lbl, 
    const std::string& sht, const std::string& tooltip, Icon* icon, 
    bool sel, bool enb, IconPressedFunc f = NULL, 
    const pxr::VtArray<pxr::VtValue> a = pxr::VtArray<pxr::VtValue>());
  ~ToolbarButton(){};
  bool Draw() override;
};

class ToolbarUI : BaseUI
{
public:
  ToolbarUI(View* parent, const std::string& name, bool vertical=false);
  ~ToolbarUI() override;

  void MouseButton(int action, int button, int mods) override {};
  void MouseMove(int x, int y) override {};
  bool Draw() override;

private:
  bool                        _vertical;
  pxr::GfVec3f                _color;
  std::vector<ToolbarItem*>   _items;
  ToolbarItem*                _current;
  static ImGuiWindowFlags     _flags;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UI_TOOLBAR_H