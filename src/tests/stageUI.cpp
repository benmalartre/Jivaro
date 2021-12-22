#include "stageUI.h"

PXR_NAMESPACE_OPEN_SCOPE

void RecurseStagePrim(GraphUI* ui, 
                      NodeUI* parent,
                      const pxr::UsdPrim& prim, 
                      int stageIndex)
{

  for(auto child : prim.GetChildren())
  {
    NodeUI* node = new NodeUI(child);
    ui->AddNode(node);
    
    RecurseStagePrim(ui, ui->GetLastNode(), child, stageIndex);
  }
}

void TestStageUI(GraphUI* ui, 
  const std::vector<pxr::UsdStageRefPtr>& stages)
{
  int numStages = stages.size();
  ui->Init(stages);
  for(int i=0;i<numStages;++i)
  {
    
    //ui->_stages[i] = new GraphStageUI(stages[i]);
    pxr::UsdPrim root = stages[i]->GetPseudoRoot();
    RecurseStagePrim(ui, NULL, root, i);
  }
}

PXR_NAMESPACE_CLOSE_SCOPE