#ifndef JVR_EMBREE_SUBDIV_H
#define JVR_EMBREE_SUBDIV_H

#include "../embree/prim.h"
#include "../embree/utils.h"
#include <pxr/usd/usdGeom/mesh.h>


JVR_NAMESPACE_OPEN_SCOPE

class UsdEmbreeContext;
struct UsdEmbreeSubdiv  : public UsdEmbreePrim {
  VtArray<GfVec3f>  _vertices;

  VtArray<int>           _indices;
  VtArray<int>           _counts;

  bool                        _hasVertexCreaseWeights;
  VtArray<float>         _vertexCreaseWeights;
  VtArray<int>           _vertexCreaseIndices;

  bool                        _hasEdgeCreaseWeights;
  VtArray<float>         _edgeCreaseWeights;
  VtArray<int>           _edgeCreaseIndices;

  bool                        _hasUVs;
  JVR_INTERPOLATION_TYPE      _uvsInterpolationType;
  VtArray<GfVec2f>  _uvs;

  bool                        _hasColors;
  JVR_INTERPOLATION_TYPE      _colorsInterpolationType;
  VtArray<GfVec3f>  _colors;

  //std::vector<UsdEmbreeMeshDatas> _extraDatas;
};

UsdEmbreeSubdiv* TranslateSubdiv( 
  UsdEmbreeContext* ctxt, 
  const UsdGeomMesh& usdMesh,
  const GfMatrix4d& worldMatrix,
  RTCScene scene
);

bool CheckNormals(
  const UsdGeomMesh& usdMesh,
  const UsdTimeCode& time,
  UsdEmbreeSubdiv* subdiv
);

bool 
CheckColors(const UsdGeomMesh& usdMesh,
            const UsdTimeCode& time,
            UsdEmbreeSubdiv* mesh);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_EMBREE_SUBDIV_H