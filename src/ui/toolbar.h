#ifndef JVR_UI_TOOLBAR_H
#define JVR_UI_TOOLBAR_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include "../ui/style.h"
#include "../ui/fonts.h"

#include <vector>
#include <pxr/base/vt/value.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>


JVR_NAMESPACE_OPEN_SCOPE

enum ToolbarItemType {
  TOOLBAR_BUTTON,
  TOOLBAR_SEPARATOR
};

// callback prototype
//typedef void(*ToolbarPressedFunc)(const VtArray<VtValue>& args);
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

  VtArray<VtValue>  args;
  CALLBACK_FN                 func;
  const char*                 icon;

  ToolbarButton(BaseUI* ui, short tool, const std::string& lbl, 
    const std::string& sht, const std::string& tooltip, const char* icon, 
    bool sel, bool enb, CALLBACK_FN f = NULL,
    const VtArray<VtValue> a = VtArray<VtValue>());
  ~ToolbarButton(){};
  bool Draw() override;
};


int ToolbarUIFixedSizeFunc(BaseUI* ui);

class ToolbarUI : BaseUI
{
public:
  ToolbarUI(View* parent, bool vertical=false);
  ~ToolbarUI() override;

  void MouseButton(int action, int button, int mods) override {};
  void MouseMove(int x, int y) override {};
  bool Draw() override;
  void Update();

  int GetFixedSize();

private:
  bool                        _vertical;
  GfVec3f                     _color;
  std::vector<ToolbarItem*>   _items;
  ToolbarItem*                _current;
  static ImGuiWindowFlags     _flags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_TOOLBAR_H