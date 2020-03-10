#include "stageUI.h"

AMN_NAMESPACE_OPEN_SCOPE

void RecurseStagePrim(AmnGraphUI* ui, 
                      const pxr::UsdPrim& prim, 
                      int stageIndex, 
                      int& index)
{
  for(auto child : prim.GetChildren())
  {
    AmnNodeUI nodeUI(child,index);
    AmnGraphStageUI* stage = ui->GetStage(stageIndex);
    
    stage->_nodes.push_back(nodeUI);
    RecurseStagePrim(ui, child, stageIndex, index);
  }
}

void TestStageUI(AmnGraphUI* ui, 
  const std::vector<pxr::UsdStageRefPtr>& stages)
{
  int numStages = stages.size();
  ui->Init(stages);
  int j = 0;
  for(int i=0;i<numStages;++i)
  {
    
    //ui->_stages[i] = new AmnGraphStageUI(stages[i]);
    pxr::UsdPrim root = stages[i]->GetPseudoRoot();
    RecurseStagePrim(ui, root, i, j);
  }
}

AMN_NAMESPACE_CLOSE_SCOPE