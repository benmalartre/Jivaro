#include <pxr/usd/sdf/types.h
#include "../graph/graph.h"


JVR_NAMESPACE_OPEN_SCOPE

pxr::SdfValueTypeName 
_GetRuntimeTypeName(pxr::SdfValueTypeName vtn)
{
  if (vtn == pxr::SdfValueTypeNames->Bool ||
    vtn == pxr::SdfValueTypeNames->BoolArray) return pxr::SdfValueTypeNames->BoolArray;
  else if (vtn == pxr::SdfValueTypeNames->Int ||
    vtn == pxr::SdfValueTypeNames->IntArray) return pxr::SdfValueTypeNames->IntArray;
  else if (vtn == pxr::SdfValueTypeNames->UChar) return pxr::SdfValueTypeNames->UCharArray;
  else if (vtn == pxr::SdfValueTypeNames->Float ||
    vtn == pxr::SdfValueTypeNames->FloatArray ||
    vtn == pxr::SdfValueTypeNames->Double ||
    vtn == pxr::SdfValueTypeNames->DoubleArray) return pxr::SdfValueTypeNames->FloatArray;
  else if (vtn == pxr::SdfValueTypeNames->Float2 ||
    vtn == pxr::SdfValueTypeNames->Float2Array) return pxr::SdfValueTypeNames->Float2Array;
  else if (vtn == pxr::SdfValueTypeNames->Float3 ||
    vtn == pxr::SdfValueTypeNames->Vector3f ||
    vtn == pxr::SdfValueTypeNames->Float3Array ||
    vtn == pxr::SdfValueTypeNames->Vector3fArray ||
    vtn == pxr::SdfValueTypeNames->Point3f ||
    vtn == pxr::SdfValueTypeNames->Point3fArray ||
    vtn == pxr::SdfValueTypeNames->Normal3f ||
    vtn == pxr::SdfValueTypeNames->Normal3fArray ||
    vtn == pxr::SdfValueTypeNames->Vector3d ||
    vtn == pxr::SdfValueTypeNames->Double3Array ||
    vtn == pxr::SdfValueTypeNames->Vector3dArray ||
    vtn == pxr::SdfValueTypeNames->Point3d ||
    vtn == pxr::SdfValueTypeNames->Point3dArray ||
    vtn == pxr::SdfValueTypeNames->Normal3d ||
    vtn == pxr::SdfValueTypeNames->Normal3dArray) return pxr::SdfValueTypeNames->Float3Array;
  else if (vtn == pxr::SdfValueTypeNames->Float4 ||
    vtn == pxr::SdfValueTypeNames->Float4Array) return pxr::SdfValueTypeNames->Float4Array;
  else if (vtn == pxr::SdfValueTypeNames->Color4f ||
    vtn == pxr::SdfValueTypeNames->Color4fArray) return pxr::SdfValueTypeNames->Color3fArray;
  else if (vtn == pxr::SdfValueTypeNames->Asset ||
    vtn == pxr::SdfValueTypeNames->AssetArray) return pxr::SdfValueTypeNames->AssetArray;
  else if (vtn == pxr::SdfValueTypeNames->Token ||
    vtn == pxr::SdfValueTypeNames->TokenArray) return pxr::SdfValueTypeNames->TokenArray;
  else
    return pxr::SdfValueTypeNames->Int;
}

JVR_NAMESPACE_CLOSE_SCOPE