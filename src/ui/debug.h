#ifndef JVR_UI_DEBUG_H
#define JVR_UI_DEBUG_H

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"
#include <pxr/usd/usd/prim.h>

JVR_NAMESPACE_OPEN_SCOPE

class DebugUI : public BaseUI
{
public:
  DebugUI(View* parent);
  ~DebugUI()         override;
  bool Draw()      override;

private:
  void _DrawDebugCodes();
  void _DrawTraceReporter();
  void _DrawPlugins();
  static ImGuiWindowFlags _flags;
  
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_DEBUG_H