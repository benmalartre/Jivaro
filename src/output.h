#pragma once

#include <pxr/usd/usdShade/output.h>

namespace AMN {
  class AmnOutputPort : public pxr::UsdShadeOutput {
  public:
    AmnOutputPort()
    {
      // nothing
    }

    AddNode();
    RemoveNode();
  private:
    pxr::UsdAttribute* ;
  };

} // namespace AMN
