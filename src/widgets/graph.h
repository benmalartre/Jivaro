#pragma once

#include "../default.h"
#include "../app/ui.h"
#include "../utils/utils.h"
#include "node.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

struct AmnGraphStageUI {
  pxr::UsdStageRefPtr               _stage;
  std::vector<AmnNodeUI>            _nodes;
  std::vector<AmnConnectionUI>      _connections;

  AmnGraphStageUI(const pxr::UsdStageRefPtr& stage):_stage(stage){};
};

class AmnGraphUI : public AmnUI
{
public:
  AmnGraphUI(AmnView* parent, const std::string& filename);
  ~AmnGraphUI()         override;

  void Event()     override;
  void Draw()      override;

  void Init(const std::vector<pxr::UsdStageRefPtr>& stages);
  AmnGraphStageUI* GetStage(int index);
  void FillBackground();
private:
  std::string                   _filename;
  pxr::GfVec3f                  _color;
  std::vector<AmnGraphStageUI*>  _stages;

};

AMN_NAMESPACE_CLOSE_SCOPE