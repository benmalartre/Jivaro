#pragma once

#include "prim.h"

namespace AMN {
  class UsdEmbreeContext;
  struct UsdEmbreeMesh  : public UsdEmbreePrim {
    unsigned                    _numOriginalSamples;
    pxr::VtArray<pxr::GfVec3f>  _positions;
    pxr::VtArray<int>           _triangles;
    pxr::VtArray<int>           _samples;

    bool                        _hasNormals;
    INTERPOLATION_TYPE          _normalsInterpolationType;
    pxr::VtArray<pxr::GfVec3f>  _normals;

    bool                        _hasUVs;
    INTERPOLATION_TYPE          _uvsInterpolationType;
    pxr::VtArray<pxr::GfVec2f>  _uvs;

    bool                        _hasColors;
    INTERPOLATION_TYPE          _colorsInterpolationType;
    pxr::VtArray<pxr::GfVec3f>  _colors;

    //std::vector<UsdEmbreeMeshDatas> _extraDatas;
  };

  UsdEmbreeMesh* TranslateMesh( UsdEmbreeContext* ctxt,
                                const pxr::UsdGeomMesh& usdMesh);

  bool CheckNormals(const pxr::UsdGeomMesh& usdMesh,
                    const pxr::UsdTimeCode& time,
                    UsdEmbreeMesh* mesh);

  void ComputeVertexNormals(const pxr::VtArray<pxr::GfVec3f>& positions,
                            const pxr::VtArray<int>& counts,
                            const pxr::VtArray<int>& indices,
                            const pxr::VtArray<int>& triangles,
                            pxr::VtArray<pxr::GfVec3f>& normals);

  int TriangulateMesh(const pxr::VtArray<int>& counts, 
                      const pxr::VtArray<int>& indices, 
                      pxr::VtArray<int>& triangles,
                      pxr::VtArray<int>& samples);

  template<typename T>
  void TriangulateData(const pxr::VtArray<int>& indices, 
                      const pxr::VtArray<T>& datas,
                      pxr::VtArray<T>& result);

} // namespace AMN
