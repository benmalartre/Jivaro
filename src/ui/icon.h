#ifndef JVR_UI_ICON_H
#define JVR_UI_ICON_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/fonts.h"

JVR_NAMESPACE_OPEN_SCOPE

class IconUI : public BaseUI
{
public:
  IconUI(View* parent);
  ~IconUI() override;
  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
  void MouseMove(int x, int y) override;
  void MouseWheel(int x, int y) override;

private:
  static ImGuiWindowFlags _flags;
  pxr::GfVec2f            _offset;
  float                   _scale;
  bool                    _drag;
  pxr::GfVec2f            _origin;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_ICON_H