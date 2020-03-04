#include "stageUI.h"

AMN_NAMESPACE_OPEN_SCOPE

void RecurseStagePrim(AmnGraphUI* ui, const pxr::UsdPrim& prim, int index)
{
  for(auto child : prim.GetChildren())
  {
    std::cout << child.GetName() << " : ";
    std::cout << child.GetTypeName().GetText() << "#### ";
    AmnNodeUI nodeUI(child);
    AmnGraphStageUI* stage = ui->GetStage(index);
    if(stage)
    {
      stage->_nodes.push_back(nodeUI);
      //ui->_stages[index]->_nodes.push_back(nodeUI);
      RecurseStagePrim(ui, child, index);
    }
    

  }
}

void TestStageUI(AmnGraphUI* ui, 
  const std::vector<pxr::UsdStageRefPtr>& stages)
{
  int numStages = stages.size();
  ui->Init(stages);
  for(int i=0;i<numStages;++i)
  {
    //ui->_stages[i] = new AmnGraphStageUI(stages[i]);
    pxr::UsdPrim root = stages[i]->GetPseudoRoot();
    RecurseStagePrim(ui, root, i);
  }
}

AMN_NAMESPACE_CLOSE_SCOPE