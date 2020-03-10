#pragma once

#include "../default.h"
#include "../app/ui.h"
#include "../utils/utils.h"
#include "node.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primFlags.h>
#include <vector>

AMN_NAMESPACE_OPEN_SCOPE

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

  void Event()     override;
  void Draw()      override;

  void Init(const std::string& filename);
  void Init(const std::vector<pxr::UsdStageRefPtr>& stages);
  void Term();

  void SetCurrentColor(int color){_color = color;};
  int GetCurrentColor(){return _color;};

  AmnGraphStageUI* GetStage(int index);
  
  void BuildGraph(int index);
  
private:

  void _RecurseStagePrim(const pxr::UsdPrim& prim, 
                        int stageIndex);

  std::string                           _filename;
  std::vector<AmnGraphStageUI*>         _stages;
  AmnGraphStageUI*                      _current;
  ImNodes::EditorContext*               _context;
  
  int                                   _color;
  int                                   _id;
  int                                   _depth;
  
};

AMN_NAMESPACE_CLOSE_SCOPE