#include "graph.h"
#include "../graph/stage.h"
#include "../app/view.h"
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/attribute.h>



AMN_NAMESPACE_OPEN_SCOPE

enum NodeType
{
    Node_Stage,
    Node_Layer,
    Node_Xform,
    Node_Mesh,
    Node_Subdiv,
    Node_Curve,
    Node_Points,
    Node_Nurbs
};

// constructor
//------------------------------------------------------------------------------
GraphUI::GraphUI(View* parent, const std::string& filename):
BaseUI(parent, "Graph")
{
  _filename = filename;
  _id = 0;
}

// destructor
//------------------------------------------------------------------------------
GraphUI::~GraphUI()
{
  for(auto stage : _stages) delete stage;
}

// term
//------------------------------------------------------------------------------
void GraphUI::Term()
{
    ImNodes::EditorContextFree(_context);
}

// draw
//------------------------------------------------------------------------------
void GraphUI::Draw()
{
  pxr::GfVec2f offset(2,2);
  ImGui::SetNextWindowPos(_parent->GetMin()+offset);
  ImGui::SetNextWindowSize(_parent->GetSize()-offset);
  //std::cout << "DRAW GRAP UI BEGIN" << std::endl;
  ImGui::Begin("Graph Editor");
  //ImGui::SetWindowPos(_parent->GetMin());
  //std::cout << "DRAW GRAP SET POSITION" << std::endl;
  //ImGui::SetWindowSize(_parent->GetSize());
  //std::cout << "DRAW GRAP SET SIZE" << std::endl;
  for(auto stage : _stages)
  {
    //std::cout << "DRAW GRAP DRAW STAGE BEGIN" << std::endl;
    ImNodes::BeginNodeEditor();
    for(auto node : stage->_nodes)
      node.Draw();
    //std::cout << "DRAW GRAP DRAW NODES" << std::endl;
    for(auto cnx : stage->_connexions)
      cnx.Draw();
    //std::cout << "DRAW GRAP DRAW CONNEXIONS" << std::endl;
    int start, end;
    ImNodes::EndNodeEditor();
    
    if (ImNodes::IsLinkCreated(&start, &end))
    {
      stage->_connexions.push_back(ConnexionUI(_id++, start, end, _color));
      //std::cout << "DRAW GRAP CREATE CONNEXION" << std::endl;
    }
    //std::cout << "DRAW GRAP DRAW STAGE END" << std::endl;
  }
  ImGui::End();
  //std::cout << "DRAW GRAP UI END" << std::endl;
};

// init
//------------------------------------------------------------------------------
void GraphUI::Init(const std::string& filename)
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
      _stages.push_back(new GraphStageUI(usdStage));
    }
  }
  _context = ImNodes::EditorContextCreate();
  
}

// init
//------------------------------------------------------------------------------
void GraphUI::Init(const std::vector<pxr::UsdStageRefPtr>& stages)
{
  if(_stages.size())
    for(auto stage: _stages) delete stage;

  _stages.resize(stages.size());
  for(int i=0;i<stages.size();++i)
  {
    _stages[i] = new GraphStageUI(stages[i]);
  }
  _context = ImNodes::EditorContextCreate();
};

// get stage at index
//------------------------------------------------------------------------------
GraphStageUI* GraphUI::GetStage(int index)
{
  if(index>=0 && index < _stages.size()) return _stages[index];
  else return NULL;
}

// update port map
//------------------------------------------------------------------------------ 
void GraphStageUI::Update(const NodeUI& node)
{
   for(const auto port : node.GetInputs()) 
      _portMap.insert(std::make_pair<int, GraphPortMapData>
        (port.GetId(), 
        (GraphPortMapData){&node, port.GetName()}));
    
    for(const auto port : node.GetOutputs()) 
      _portMap.insert(std::make_pair<int, GraphPortMapData>
        (port.GetId(), 
        (GraphPortMapData){&node, port.GetName()}));
    
    _nodes.push_back(node);
}

// build graph
//------------------------------------------------------------------------------ 
void GraphUI::_RecurseStagePrim(const pxr::UsdPrim& prim, int stageIndex)
{
  for(auto child : prim.GetChildren())
  {
    NodeUI nodeUI(child, _id);
    GraphStageUI* stage = GetStage(stageIndex);

    stage->Update(nodeUI);
    
    _RecurseStagePrim(child, stageIndex);
  }
}

void GraphUI::BuildGraph(int stageIndex)
{
  GraphStageUI* stage = GetStage(stageIndex);
  if(stage)
  {
    int nodeIndex = 0;
    _RecurseStagePrim(stage->_stage->GetPseudoRoot(), stageIndex);
  }
}

void GraphUI::MouseButton(int action, int button, int mods)
{

}

void GraphUI::MouseMove(int x, int y)
{

}

AMN_NAMESPACE_CLOSE_SCOPE