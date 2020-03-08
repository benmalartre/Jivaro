#include "graph.h"
#include "../graph/stage.h"
#include "../app/view.h"
#include <pxr/usd/usd/primRange.h>


AMN_NAMESPACE_OPEN_SCOPE

// constructor
//------------------------------------------------------------------------------
AmnGraphUI::AmnGraphUI(AmnView* parent, const std::string& filename):
AmnUI(parent, "Graph")
{
  _filename = filename;
  _nodeId = 0;
}

// destructor
//------------------------------------------------------------------------------
AmnGraphUI::~AmnGraphUI(){}

// event
//------------------------------------------------------------------------------
void AmnGraphUI::Event()
{
  std::cerr << "AmnGraphUI EVENT!" << std::endl;
};

// term
//------------------------------------------------------------------------------
void AmnGraphUI::Term()
{
    ImNodes::EditorContextFree(_context);
}

// draw
//------------------------------------------------------------------------------
void AmnGraphUI::Draw()
{
  ImGui::Begin("Graph Editor");
  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::SetWindowSize(_parent->GetSize());

  ImNodes::BeginNodeEditor();

  for(auto stage : _stages)
    for(auto node : stage->_nodes)
      node.Draw();

  ImNodes::EndNodeEditor();
  ImGui::End();

  /*
  bool opened;
  int flags = 0;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoMove;
  

  ImGui::Begin(_name.c_str(), &opened, flags);
  pxr::GfVec4f color(
    RANDOM_0_1,
    RANDOM_0_1,
    RANDOM_0_1, 
    1.f
  );

  //ImGui::TestDummyView(&opened, _parent->GetMin(), _parent->GetMax(), color);
  ImGui::TestGraphNodes(&opened, _parent->GetMin(), _parent->GetMax());
  ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::End();
  */
};

// init
//------------------------------------------------------------------------------
void AmnGraphUI::Init(const std::string& filename)
{
  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filename);
  pxr::UsdPrimRange primRange = stage->Traverse();
  for(auto prim: primRange)
  {
    if(prim.GetTypeName() == pxr::TfToken("Stage"))
    {
      pxr::GraphStage stageNode(prim);
      std::string fileName;
      pxr::UsdAttribute fileNameAttr = stageNode.CreateFileNameAttr();
      fileNameAttr.Get(&fileName);

      pxr::UsdStageRefPtr usdStage = pxr::UsdStage::CreateInMemory();
      _stages.push_back(new AmnGraphStageUI(usdStage));
    }
  }
  _context = ImNodes::EditorContextCreate();
  
}

// init
//------------------------------------------------------------------------------
void AmnGraphUI::Init(const std::vector<pxr::UsdStageRefPtr>& stages)
{
  if(_stages.size())
    for(auto stage: _stages) delete stage;

  _stages.resize(stages.size());
  for(int i=0;i<stages.size();++i)
  {
    _stages[i] = new AmnGraphStageUI(stages[i]);
  }
  _context = ImNodes::EditorContextCreate();
};

// get stage at index
//------------------------------------------------------------------------------
AmnGraphStageUI* AmnGraphUI::GetStage(int index)
{
  if(index>=0 && index < _stages.size()) return _stages[index];
  else return NULL;
}

// build graph
//------------------------------------------------------------------------------ 
void AmnGraphUI::_RecurseStagePrim(const pxr::UsdPrim& prim, 
                        int stageIndex, 
                        int& nodeIndex)
{
  for(auto child : prim.GetChildren())
  {
    AmnNodeUI nodeUI(child, nodeIndex);
    nodeIndex++;
    AmnGraphStageUI* stage = GetStage(stageIndex);
    
    stage->_nodes.push_back(nodeUI);
    _RecurseStagePrim(child, stageIndex, nodeIndex);
  }
}

void AmnGraphUI::BuildGraph(int index)
{
  AmnGraphStageUI* stage = GetStage(index);
  if(stage)
  {
    int nodeIndex = 0;
    _RecurseStagePrim(stage->_stage->GetPseudoRoot(), index, nodeIndex);
  }
}

AMN_NAMESPACE_CLOSE_SCOPE