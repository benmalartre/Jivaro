#pragma once

#include "../default.h"
#include "../app/ui.h"
#include "../utils/utils.h"
#include "node.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primFlags.h>
#include <vector>

AMN_NAMESPACE_OPEN_SCOPE

enum ColorGraph {
  GRAPH_COLOR_UNDEFINED         = 0xFF000000,
  GRAPH_COLOR_BOOL              = 0xFFFF6600,
  GRAPH_COLOR_INTEGER           = 0xFF336611,
  GRAPH_COLOR_ENUM              = 0xFF339911,
  GRAPH_COLOR_FLOAT             = 0xFF33CC33,
  GRAPH_COLOR_VECTOR2           = 0xFFFFCC00,
  GRAPH_COLOR_VECTOR3           = 0xFFFFFF00,
  GRAPH_COLOR_VECTOR4           = 0xFFFFFF66,
  GRAPH_COLOR_COLOR             = 0xFFFF0000, 
  GRAPH_COLOR_ROTATION          = 0xFFCCFFFF,
  GRAPH_COLOR_QUATERNION        = 0xFF66FFFF,
  GRAPH_COLOR_MATRIX3           = 0xFF00FFFF,
  GRAPH_COLOR_MATRIX4           = 0xFF33CCFF,
  GRAPH_COLOR_STRING            = 0xFFCC99FF,
  GRAPH_COLOR_SHAPE             = 0xFFFF3399,
  GRAPH_COLOR_TOPOLOGY          = 0xFFCCCCCC,
  GRAPH_COLOR_GEOMETRY          = 0xFFFF3366,
  GRAPH_COLOR_LOCATION          = 0xFF555577,
  GRAPH_COLOR_CONTOUR           = 0xFF000000
};

struct AmnGraphPortMapData {
  const AmnNodeUI*                        _node;
  std::string                             _port;
};

struct AmnGraphStageUI {
  int                                     _nodeId;
  pxr::UsdStageRefPtr                     _stage;
  std::vector<AmnNodeUI>                  _nodes;
  std::vector<AmnConnexionUI>             _connexions;
  std::map<int, AmnGraphPortMapData>      _portMap;

  AmnGraphStageUI(const pxr::UsdStageRefPtr& stage):_stage(stage){};
  void Update(const AmnNodeUI& node);
};

class AmnGraphUI : public AmnUI
{
public:
  AmnGraphUI(AmnView* parent, const std::string& filename);
  ~AmnGraphUI()         override;

  void MouseButton(int action, int button, int mods) override;
  void MouseMove(int x, int y) override;
  void Draw()      override;

  void Init(const std::string& filename);
  void Init(const std::vector<pxr::UsdStageRefPtr>& stages);
  void Term();

  void SetCurrentColor(int color){_color = color;};
  int GetCurrentColor(){return _color;};

  AmnGraphStageUI* GetStage(int index);
  
  void BuildGraph(int index);
  
private:

  void _RecurseStagePrim(const pxr::UsdPrim& prim, int stageIndex);

  std::string                           _filename;
  std::vector<AmnGraphStageUI*>         _stages;
  AmnGraphStageUI*                      _current;
  ImNodes::EditorContext*               _context;
  
  int                                   _color;
  int                                   _id;
  int                                   _depth;
  pxr::GfVec2f                          _position;
  
};

AMN_NAMESPACE_CLOSE_SCOPE