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

void AmnNodeUI::Draw()
{

} 

AMN_NAMESPACE_CLOSE_SCOPE