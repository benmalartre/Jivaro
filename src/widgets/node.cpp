#include "node.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

AmnPortUI::AmnPortUI(const pxr::GraphInput& port, int index)
{
  _id = index;
  _color = GRAPH_COLOR_FLOAT;
  _label = port.GetBaseName().GetText();
  _io = true;
}

AmnPortUI::AmnPortUI(const pxr::GraphOutput& port, int index)
{
  _id = index;
  _color = GRAPH_COLOR_FLOAT;
  _label = port.GetBaseName().GetText();
  _io = false;
}

void AmnPortUI::Draw()
{
  if(_io)
    ImNodes::BeginInputAttribute(_id, ImNodes::PinShape_CircleFilled, _color);
  else
    ImNodes::BeginOutputAttribute(_id, ImNodes::PinShape_CircleFilled, _color);

  ImGui::Text(_label.c_str());
  ImNodes::EndAttribute();
}

// AmnConnexionUI draw
//------------------------------------------------------------------------------
void AmnConnexionUI::Draw()
{
  ImNodes::Link(_id, _start, _end, _color);
}

// AmnNodeUI constructor
//------------------------------------------------------------------------------
AmnNodeUI::AmnNodeUI(const pxr::UsdPrim& prim, int& id):
_prim(prim), _id(id)
{
  id++;
  if(_prim.IsValid())
  {
    _name = prim.GetName();
    if(_prim.IsA<pxr::GraphNode>())
    {
      pxr::GraphNode node(_prim);
      _inputs.resize(node.NumInputs());
      for(int i=0;i<_inputs.size();++i)
      {
        _inputs[i] = AmnPortUI(node.GetInput(i), id++);
      }

      int offset = _inputs.size();
      _outputs.resize(node.NumOutputs());
      for(int i=0;i<_outputs.size();++i)
      {
        _outputs[i] = AmnPortUI(node.GetOutput(i), id++);
      }
    }
    else if(_prim.IsA<pxr::GraphGraph>())
    {
      //std::cout << "PRIM IS A GRAPH :D" << std::endl;
    }
    else
    {
      //std::cout << "PRIM IS A : " << _prim.GetTypeName().GetText() << std::endl;
    }
    
  }
}

// AmnNodeUI destructor
//------------------------------------------------------------------------------
AmnNodeUI::~AmnNodeUI()
{

}

void AmnNodeUI::Update()
{
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  pxr::UsdAttribute posAttr = api.GetPosAttr();
  if(posAttr && posAttr.HasAuthoredValue())
  {
    posAttr.Get(&_pos);
  }

  pxr::UsdAttribute sizeAttr = api.GetSizeAttr();
  if(sizeAttr && sizeAttr.HasAuthoredValue())
  {
    sizeAttr.Get(&_size);
  }
  else 
  {
    _size[0] = 128;
    _size[1] = 64;
  }

  _size[1] += _inputs.size() * NODE_PORT_SPACING;

}

// AmnNodeUI draw
//------------------------------------------------------------------------------
void AmnNodeUI::Draw()
{
  Update();
  ImGui::SetCursorPos(GetPos());
  ImNodes::BeginNode(_id);

  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(_name.c_str());
  ImNodes::EndNodeTitleBar();

  int numInputs = _inputs.size();
  
  for(int i=0;i<numInputs;++i) _inputs[i].Draw();

  int numOutputs = _outputs.size();
  ImGui::Indent(40);
  for(int i=0;i<numOutputs;++i) _outputs[i].Draw();

  ImNodes::EndNode();
} 

AMN_NAMESPACE_CLOSE_SCOPE