#ifndef JVR_UI_DEMO_H
#define JVR_UI_DEMO_H

#include "../common.h"
#include "../ui/head.h"
#include "../ui/utils.h"
#include <pxr/usd/usd/prim.h>

JVR_NAMESPACE_OPEN_SCOPE

class DemoUI : public HeadedUI
{
public:
  DemoUI(View* parent);
  ~DemoUI()         override;
  bool Draw()      override;

private:
  static ImGuiWindowFlags _flags;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_DEBUG_H