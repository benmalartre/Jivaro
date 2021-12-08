#ifndef AMN_EMBREE_SUBDIV_H
#define AMN_EMBREE_SUBDIV_H

#include "../embree/prim.h"
#include "../embree/utils.h"
#include <pxr/usd/usdGeom/mesh.h>


AMN_NAMESPACE_OPEN_SCOPE

class UsdEmbreeContext;
struct UsdEmbreeSubdiv  : public UsdEmbreePrim {
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

UsdEmbreeSubdiv* TranslateSubdiv( 
  UsdEmbreeContext* ctxt, 
  const pxr::UsdGeomMesh& usdMesh,
  const pxr::GfMatrix4d& worldMatrix,
  RTCScene scene
);

bool CheckNormals(
  const pxr::UsdGeomMesh& usdMesh,
  const pxr::UsdTimeCode& time,
  UsdEmbreeSubdiv* subdiv
);

bool 
CheckColors(const pxr::UsdGeomMesh& usdMesh,
            const pxr::UsdTimeCode& time,
            UsdEmbreeSubdiv* mesh);

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_EMBREE_SUBDIV_H