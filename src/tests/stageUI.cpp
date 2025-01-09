#include "stageUI.h"

JVR_NAMESPACE_OPEN_SCOPE

void RecurseStagePrim(GraphEditorUI* ui, 
                      NodeUI* parent,
                      const UsdPrim& prim, 
                      int stageIndex)
{

  for(auto child : prim.GetChildren())
  {
    NodeUI* node = new NodeUI(child);
    ui->AddNode(node);
    
    RecurseStagePrim(ui, ui->GetLastNode(), child, stageIndex);
  }
}

void TestStageUI(GraphEditorUI* ui, 
  const std::vector<UsdStageRefPtr>& stages)
{
  int numStages = stages.size();
  ui->Init(stages);
  for(int i=0;i<numStages;++i)
  {
    
    //ui->_stages[i] = new GraphStageUI(stages[i]);
    UsdPrim root = stages[i]->GetPseudoRoot();
    RecurseStagePrim(ui, NULL, root, i);
  }
}

JVR_NAMESPACE_CLOSE_SCOPE