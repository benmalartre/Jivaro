#ifndef JVR_UI_COMMANDS_H
#define JVR_UI_COMMANDS_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/fonts.h"

JVR_NAMESPACE_OPEN_SCOPE

class CommandsUI : public BaseUI
{
public:
  CommandsUI(View* parent);
  ~CommandsUI()         override;
  bool Draw()      override;

private:
  static ImGuiWindowFlags _flags;
  
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_TOOL_H