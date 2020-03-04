#include "node.h"
#include <pxr/usd/usd/prim.h>

AMN_NAMESPACE_OPEN_SCOPE

void AmnPortUI::Draw()
{

}

void AmnConnectionUI::Draw()
{
  
}

AmnNodeUI::AmnNodeUI(const pxr::UsdPrim& prim):_prim(prim)
{
  if(_prim.IsValid())
  {
    if(_prim.IsA<pxr::GraphNode>())
    {
      std::cout << "PRIM IS A NODE :D" << std::endl;
    }
    else if(_prim.IsA<pxr::GraphGraph>())
    {
      std::cout << "PRIM IS A GRAPH :D" << std::endl;
    }
    else
    {
      std::cout << "PRIM IS A : " << _prim.GetTypeName().GetText() << std::endl;
    }
    
  }
}

AmnNodeUI::~AmnNodeUI()
{

}

pxr::GfRange2f AmnNodeUI::GetRange()
{
  pxr::UsdUINodeGraphNodeAPI api(_prim);
  pxr::UsdAttribute posAttr = api.GetPosAttr();
  pxr::GfVec2f pos;
  if(posAttr && posAttr.HasAuthoredValue())
  {
    posAttr.Get(&pos);
  }
  //std::cout << "NODE POSITION : " << pos << std::endl;

  pxr::UsdAttribute sizeAttr = api.GetSizeAttr();
  pxr::GfVec2f size;
  if(sizeAttr && sizeAttr.HasAuthoredValue())
  {
    sizeAttr.Get(&size);
  }
  else size = pxr::GfVec2f(128, 64);
  //std::cout << "NODE SIZE : " << size << std::endl;

  return pxr::GfRange2f(pos, pos + size);
}
void AmnNodeUI::Draw()
{

} 

AMN_NAMESPACE_CLOSE_SCOPE