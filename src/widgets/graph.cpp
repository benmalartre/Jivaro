#include "graph.h"
#include "../graph/stage.h"
#include "../app/view.h"
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/attribute.h>



AMN_NAMESPACE_OPEN_SCOPE

// constructor
//------------------------------------------------------------------------------
AmnGraphUI::AmnGraphUI(AmnView* parent, const std::string& filename):
AmnUI(parent, "Graph")
{
  _filename = filename;
  _id = 0;
}

// destructor
//------------------------------------------------------------------------------
AmnGraphUI::~AmnGraphUI()
{
  for(auto stage : _stages) delete stage;
}

// event
//------------------------------------------------------------------------------
void AmnGraphUI::Event()
{

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

  for(auto stage : _stages)
  {
    ImNodes::BeginNodeEditor();
    for(auto node : stage->_nodes)
      node.Draw();
    for(auto cnx : stage->_connexions)
      cnx.Draw();

    int start, end;
    ImNodes::EndNodeEditor();
    
    if (ImNodes::IsLinkCreated(&start, &end))
    {
      stage->_connexions.push_back(AmnConnexionUI(_id++, start, end, _color));
    }
  }
  ImGui::End();
  
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

// update port map
//------------------------------------------------------------------------------ 
void AmnGraphStageUI::Update(const AmnNodeUI& node)
{
   for(const auto port : node.GetInputs()) 
      _portMap.insert(std::make_pair<int, AmnGraphPortMapData>
        (port.GetId(), 
        (AmnGraphPortMapData){&node, port.GetName()}));
    
    for(const auto port : node.GetOutputs()) 
      _portMap.insert(std::make_pair<int, AmnGraphPortMapData>
        (port.GetId(), 
        (AmnGraphPortMapData){&node, port.GetName()}));
    
    _nodes.push_back(node);
}

// build graph
//------------------------------------------------------------------------------ 
void AmnGraphUI::_RecurseStagePrim(const pxr::UsdPrim& prim, int stageIndex)
{
  for(auto child : prim.GetChildren())
  {
    AmnNodeUI nodeUI(child, _id);
    AmnGraphStageUI* stage = GetStage(stageIndex);

    stage->Update(nodeUI);
    
    _RecurseStagePrim(child, stageIndex);
  }
}

void AmnGraphUI::BuildGraph(int stageIndex)
{
  AmnGraphStageUI* stage = GetStage(stageIndex);
  if(stage)
  {
    int nodeIndex = 0;
    _RecurseStagePrim(stage->_stage->GetPseudoRoot(), stageIndex);
  }
}

AMN_NAMESPACE_CLOSE_SCOPE