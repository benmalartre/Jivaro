#include "node.h"
#include "graph.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>

AMN_NAMESPACE_OPEN_SCOPE

int GetColorFromAttribute(const pxr::UsdAttribute& attr)
{
  /*
  enum ColorGraph {
  GRAPH_COLOR_UNDEFINED         = 0xFF000000,
  GRAPH_COLOR_BOOL              = 0xFFFF6600,
  GRAPH_COLOR_INTEGER           = 0xFF336611,
  GRAPH_COLOR_ENUM              = 0xFF339911,
  GRAPH_COLOR_FLOAT             = 0xFF33CC33,
  GRAPH_COLOR_VECTOR2           = 0xFFFFCC00,
  GRAPH_COLOR_VECTOR3           = 0xFFFFFF00,
  GRAPH_COLOR_VECTOR4           = 0xFFFFFF66,
  GRAPH_COLOR_COLOR             = 0xFFFF0000, 
  GRAPH_COLOR_ROTATION          = 0xFFCCFFFF,
  GRAPH_COLOR_QUATERNION        = 0xFF66FFFF,
  GRAPH_COLOR_MATRIX3           = 0xFF00FFFF,
  GRAPH_COLOR_MATRIX4           = 0xFF33CCFF,
  GRAPH_COLOR_STRING            = 0xFFCC99FF,
  GRAPH_COLOR_SHAPE             = 0xFFFF3399,
  GRAPH_COLOR_TOPOLOGY          = 0xFFCCCCCC,
  GRAPH_COLOR_GEOMETRY          = 0xFFFF3366,
  GRAPH_COLOR_LOCATION          = 0xFF555577,
  GRAPH_COLOR_CONTOUR           = 0xFF000000
  */
  pxr::SdfValueTypeName vtn = attr.GetTypeName();
  if(vtn == pxr::SdfValueTypeNames->Bool) return GRAPH_COLOR_BOOL;
  else if(vtn == pxr::SdfValueTypeNames->Int) return GRAPH_COLOR_INTEGER;
  else if(vtn == pxr::SdfValueTypeNames->UChar) return GRAPH_COLOR_ENUM;
  else if(vtn == pxr::SdfValueTypeNames->Float) return GRAPH_COLOR_FLOAT;
  else if(vtn == pxr::SdfValueTypeNames->Float2) return GRAPH_COLOR_VECTOR2;
  else if(vtn == pxr::SdfValueTypeNames->Float3) return GRAPH_COLOR_VECTOR3;
  else if(vtn == pxr::SdfValueTypeNames->Float4) return GRAPH_COLOR_VECTOR4;
  else if(vtn == pxr::SdfValueTypeNames->Color4f) return GRAPH_COLOR_COLOR;
}


//, const pxr::UsdAttribute& attr
PortUI::PortUI(const pxr::GraphInput& port, int index)
{
  _id = index;
  //SdfValueTypeName
  std::cout << "PORT TYPE NAME : " << port.GetTypeName() << std::endl;
  _color = GetColorFromAttribute(pxr::UsdAttribute(port));
  _label = port.GetBaseName().GetText();
  _io = true;

}

PortUI::PortUI(const pxr::GraphOutput& port, int index)
{
  _id = index;
  _color = GetColorFromAttribute(pxr::UsdAttribute(port));
  _label = port.GetBaseName().GetText();
  _io = false;
}

void PortUI::Draw()
{
  if(_io)
    ImNodes::BeginInputAttribute(_id, ImNodes::PinShape_CircleFilled);
  else
    ImNodes::BeginOutputAttribute(_id, ImNodes::PinShape_CircleFilled);

  ImGui::Text(_label.c_str());
  ImNodes::EndAttribute();
}

// ConnexionUI draw
//------------------------------------------------------------------------------
void ConnexionUI::Draw()
{
  ImNodes::Link(_id, _start, _end);
}

// NodeUI constructor
//------------------------------------------------------------------------------
NodeUI::NodeUI(const pxr::UsdPrim& prim, int& id):
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
        _inputs[i] = PortUI(node.GetInput(i), id++);
      }

      int offset = _inputs.size();
      _outputs.resize(node.NumOutputs());
      for(int i=0;i<_outputs.size();++i)
      {
        _outputs[i] = PortUI(node.GetOutput(i), id++);
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

// NodeUI destructor
//------------------------------------------------------------------------------
NodeUI::~NodeUI()
{

}

void NodeUI::Update()
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

// NodeUI draw
//------------------------------------------------------------------------------
void NodeUI::Draw()
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