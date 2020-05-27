#ifndef IMGUI_GRAPH_H
#define IMGUI_GRAPH_H
#pragma once

#include <stddef.h>
#include <vector>
#include <pxr/base/gf/vec2f.h>

struct ImVec2;

namespace ImGraph
{
  enum Type {
    PORT_TYPE_INPUT,
    PORT_TYPE_OUTPUT,
    PORT_TYPE_IO
  };

  class ImPort {
  public: 
    
    //void Draw();
  private:
    Type _type;
    bool _isArray;
  };

  class ImNode {
  public:
    void Draw();

  private:
    std::vector<ImPort> _inputs;
    std::vector<ImPort> _outputs;

  };

  class ImGraph {
  public:
    void Draw();

  private:
    std::vector<ImNode> _nodes;
    pxr::GfVec2f        _offset;
    float               _zoom;
  };
} 

#endif // IMGUI_GRAPH_H