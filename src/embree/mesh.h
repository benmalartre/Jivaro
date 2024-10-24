#pragma once

#include "prim.h"
#include <pxr/usd/usdGeom/mesh.h>

JVR_NAMESPACE_OPEN_SCOPE

class UsdEmbreeContext;
struct UsdEmbreeMesh  : public UsdEmbreePrim {
  unsigned                    _numOriginalSamples;
  VtArray<GfVec3f>  _vertices;
  VtArray<int>           _triangles;
  VtArray<int>           _samples;
  GfVec3d                _displayColor;

  bool                        _hasNormals;
  JVR_INTERPOLATION_TYPE      _normalsInterpolationType;
  VtArray<GfVec3f>  _normals;

  bool                        _hasUVs;
  JVR_INTERPOLATION_TYPE      _uvsInterpolationType;
  VtArray<GfVec2f>  _uvs;

  bool                        _hasColors;
  JVR_INTERPOLATION_TYPE      _colorsInterpolationType;
  VtArray<GfVec3f>  _colors;

  //std::vector<UsdEmbreeMeshDatas> _extraDatas;
};

UsdEmbreeMesh* TranslateMesh( 
  UsdEmbreeContext* ctxt, 
  const UsdGeomMesh& usdMesh,
  const GfMatrix4d& worldMatrix,
  RTCScene scene
);

void DeleteMesh(RTCScene scene, UsdEmbreeMesh* mesh);

bool CheckNormals(
  const UsdGeomMesh& usdMesh,
  const UsdTimeCode& time,
  UsdEmbreeMesh* mesh
);

bool CheckColors(
  const UsdGeomMesh& usdMesh,
  const UsdTimeCode& time,
  UsdEmbreeMesh* mesh
);
JVR_NAMESPACE_CLOSE_SCOPE