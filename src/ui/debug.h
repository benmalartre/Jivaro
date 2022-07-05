#ifndef JVR_UI_DEBUG_H
#define JVR_UI_DEBUG_H

#include "../common.h"
#include "../ui/head.h"
#include "../ui/utils.h"
#include <pxr/usd/usd/prim.h>

JVR_NAMESPACE_OPEN_SCOPE

class DebugUI : public HeadedUI
{
public:
  DebugUI(View* parent);
  ~DebugUI()         override;
  bool Draw()      override;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_DEBUG_H