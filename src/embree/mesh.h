#pragma once

#include "prim.h"
//#include "../utils/utils.h"

AMN_NAMESPACE_OPEN_SCOPE

class UsdEmbreeContext;
struct UsdEmbreeMesh  : public UsdEmbreePrim {
  unsigned                    _numOriginalSamples;
  pxr::VtArray<pxr::GfVec3f>  _vertices;
  pxr::VtArray<int>           _triangles;
  pxr::VtArray<int>           _samples;

  bool                        _hasNormals;
  AMN_INTERPOLATION_TYPE      _normalsInterpolationType;
  pxr::VtArray<pxr::GfVec3f>  _normals;

  bool                        _hasUVs;
  AMN_INTERPOLATION_TYPE      _uvsInterpolationType;
  pxr::VtArray<pxr::GfVec2f>  _uvs;

  bool                        _hasColors;
  AMN_INTERPOLATION_TYPE      _colorsInterpolationType;
  pxr::VtArray<pxr::GfVec3f>  _colors;

  //std::vector<UsdEmbreeMeshDatas> _extraDatas;
};

UsdEmbreeMesh* TranslateMesh( 
  UsdEmbreeContext* ctxt, 
  const pxr::UsdGeomMesh& usdMesh,
  const pxr::GfMatrix4d& worldMatrix,
  RTCScene scene
);

void DeleteMesh(RTCScene scene, UsdEmbreeMesh* mesh);

bool CheckNormals(
  const pxr::UsdGeomMesh& usdMesh,
  const pxr::UsdTimeCode& time,
  UsdEmbreeMesh* mesh
);

AMN_NAMESPACE_CLOSE_SCOPE