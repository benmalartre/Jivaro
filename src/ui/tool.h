#ifndef JVR_UI_TOOL_H
#define JVR_UI_TOOL_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/fonts.h"

JVR_NAMESPACE_OPEN_SCOPE

class ToolUI : public BaseUI
{
public:
  ToolUI(View* parent);
  ~ToolUI()         override;
  bool Draw()      override;

private:
  static ImGuiWindowFlags _flags;
  
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_TOOL_H