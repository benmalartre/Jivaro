#pragma once

#include "prim.h"

namespace AMN {
  extern RTCScene g_scene;
  extern RTCDevice g_device;

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

  UsdEmbreeMesh* TranslateMesh( RTCDevice device,
                                RTCScene scene,
                                const pxr::UsdGeomMesh& usdMesh, 
                                double time);

  bool CheckNormals(const pxr::UsdGeomMesh& usdMesh,
                    double time,
                    UsdEmbreeMesh* mesh);

  int TriangulateMesh(const pxr::VtArray<int>& counts, 
                      const pxr::VtArray<int>& indices, 
                      pxr::VtArray<int>& triangles,
                      pxr::VtArray<int>& samples);

  template<typename T>
  void TriangulateData(const pxr::VtArray<int>& indices, 
                      const pxr::VtArray<T>& datas,
                      pxr::VtArray<T>& result);

} // namespace AMN
