#pragma once

#include "../default.h"
#include "../ui.h"
#include <pxr/base/gf/vec3f.h>

namespace AMN {
  class NodePort
  {

  }

  class Node : UI
  {
  public: 
    Node();
    ~Node();
  private:
    pxr::GfVec3f _color;
    std::string _name;


  };

} // namespace AMN
