#pragma once

#include "../default.h"
#include "../app/ui.h"
#include "../graph/node.h"
#include "../graph/graph.h"
#include "../imgui/imgui_nodes.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/range2f.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>

AMN_NAMESPACE_OPEN_SCOPE
enum GRAPH_COLORS {
  GRAPH_COLOR_UNDEFINED         = 0xFF000000,
  GRAPH_COLOR_BOOL              = 0xFFFF6600,
  GRAPH_COLOR_FLOAT             = 0xFF33CC33,
  GRAPH_COLOR_CONTOUR           = 0xFF000000,

  /*
  #ATTR_COLOR_BOOL              = $0066FF
  #ATTR_COLOR_INTEGER           = $116633
  #ATTR_COLOR_ENUM              = $119933
  #ATTR_COLOR_FLOAT             = $33CC33
  #ATTR_COLOR_VECTOR2           = $00CCFF
  #ATTR_COLOR_VECTOR3           = $00FFFF
  #ATTR_COLOR_VECTOR4           = $66FFFF
  #ATTR_COLOR_COLOR             = $0000FF
  #ATTR_COLOR_ROTATION          = $FFFFCC
  #ATTR_COLOR_QUATERNION        = $FFFF66
  #ATTR_COLOR_MATRIX3           = $FFFF00
  #ATTR_COLOR_MATRIX4           = $FFCC33
  #ATTR_COLOR_STRING            = $FF99CC
  #ATTR_COLOR_SHAPE             = $9933FF
  #ATTR_COLOR_TOPOLOGY          = $CCCCCC
  #ATTR_COLOR_GEOMETRY          = $6633FF
  #ATTR_COLOR_LOCATION          = $775555
  #ATTR_COLOR_EXECUTE           = $777777
  #ATTR_COLOR_REFERENCE         = $CC6611
  #ATTR_COLOR_FRAMEBUFFER       = $FF6600
  #ATTR_COLOR_TEXTURE           = $FF8844       
  #ATTR_COLOR_UNIFORM           = $FFCCAA 
  #ATTR_COLOR_SHADER            = $FFFFCC
  #ATTR_COLOR_3DOBJECT          = $00DDFF 
  #ATTR_COLOR_AUDIO             = $AA22CC
  #ATTR_COLOR_FILE              = $FF9933
  #ATTR_COLOR_CUSTOM            = $DDDDDD
  
  #ATTR_COLOR_BACKGROUND        = $666666
  #ATTR_COLOR_InputBackground   = $999999
  #ATTR_COLOR_InputEdit         = $FFFFFF
  #ATTR_COLOR_BorderUnselected  = $222222
  #ATTR_COLOR_BorderSelected    = $999999
  #ATTR_COLOR_SliderLeft        = $888888
  #ATTR_COLOR_SliderRight       = $555555
  #ATTR_COLOR_Text              = $111111
  #ATTR_COLOR_Title             = $221111
  #ATTR_COLOR_TitleBackground   = 1973790
  #ATTR_COLOR_PropertyBackground= 3289650
  */
};

class AmnNodeUI;

#define NODE_CORNER_ROUNDING    4.f
#define NODE_PORT_RADIUS        4
#define NODE_PORT_SPACING       12
#define NODE_HEADER_HEIGHT      32

class AmnPortUI {
public:
  AmnPortUI(){};
  AmnPortUI(const pxr::GraphInput& port, int index);
  AmnPortUI(const pxr::GraphOutput& port, int index, int offset);
  void Draw();

private:
  int                   _id;
  std::string           _label;
  int                   _color;
  pxr::GfVec2f          _pos;
  bool                  _io;
};

class AmnConnectionUI {
public:
  void Draw();
private:
  AmnPortUI*            _start;
  AmnPortUI*            _end;
  unsigned              _color;
};

class AmnNodeUI
{
public: 
  AmnNodeUI(const pxr::UsdPrim& prim, int id);
  ~AmnNodeUI();

  const pxr::GfVec2f& GetPos() const {return _pos;};
  const pxr::GfVec2f& GetSize() const {return _size;};
  void Update();
  void Draw();

private:
  int                         _id;
  pxr::GfVec2f                _pos;
  pxr::GfVec2f                _size;
  pxr::GfVec3f                _color;
  std::string                 _name;
  pxr::UsdPrim                _prim;
  std::vector<AmnPortUI>      _inputs;
  std::vector<AmnPortUI>      _outputs;
  std::vector<AmnPortUI>      _overrides;

};

AMN_NAMESPACE_CLOSE_SCOPE