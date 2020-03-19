#pragma once

#include "prim.h"
//#include "../utils/utils.h"

AMN_NAMESPACE_OPEN_SCOPE

class AmnUsdEmbreeContext;
struct AmnUsdEmbreeSubdiv  : public AmnUsdEmbreePrim {
  pxr::VtArray<pxr::GfVec3f>  _vertices;

  pxr::VtArray<int>           _indices;
  pxr::VtArray<int>           _counts;

  bool                        _hasVertexCreaseWeights;
  pxr::VtArray<float>         _vertexCreaseWeights;
  pxr::VtArray<int>           _vertexCreaseIndices;

  bool                        _hasEdgeCreaseWeights;
  pxr::VtArray<float>         _edgeCreaseWeights;
  pxr::VtArray<int>           _edgeCreaseIndices;

  bool                        _hasUVs;
  AMN_INTERPOLATION_TYPE      _uvsInterpolationType;
  pxr::VtArray<pxr::GfVec2f>  _uvs;

  bool                        _hasColors;
  AMN_INTERPOLATION_TYPE      _colorsInterpolationType;
  pxr::VtArray<pxr::GfVec3f>  _colors;

  //std::vector<UsdEmbreeMeshDatas> _extraDatas;
};

AmnUsdEmbreeSubdiv* TranslateSubdiv( 
  AmnUsdEmbreeContext* ctxt, 
  const pxr::UsdGeomMesh& usdMesh,
  const pxr::GfMatrix4d& worldMatrix
);

bool CheckNormals(
  const pxr::UsdGeomMesh& usdMesh,
  const pxr::UsdTimeCode& time,
  AmnUsdEmbreeSubdiv* subdiv
);

AMN_NAMESPACE_CLOSE_SCOPE