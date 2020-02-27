#pragma once

#include <embree3/rtcore.h>

#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <math/vec3.h>

namespace embree {
  extern RTCScene g_scene;
  extern Vec3fa* face_colors; 
  extern Vec3fa* vertex_colors;
  extern RTCDevice g_device;
  extern bool g_changed;
  extern float g_debug;

  enum INTERPOLATION_MODE{
    CONSTANT,
    UNIFORM,
    VARYING,
    VERTEX,
    FACE_VARYING
  };

  template<typename T>
  struct UsdEmbreeMeshDatas {
    INTERPOLATION_MODE          _interpolationMode;
    pxr::VtArray<T>             _datas;
  };

  struct UsdEmbreeMesh {
    unsigned                    _geomId;
    RTCGeometry                 _mesh;
    pxr::VtArray<pxr::GfVec3f>  _positions;
    pxr::VtArray<int>           _triangles;
    pxr::VtArray<int>           _samples;

    bool                        _hasNormals;
    INTERPOLATION_MODE          _normalsInterpolationMode;
    pxr::VtArray<pxr::GfVec3f>  _normals;

    bool                        _hasUVs;
    INTERPOLATION_MODE          _uvsInterpolationMode;
    pxr::VtArray<pxr::GfVec2f>  _uvs;

    bool                        _hasColors;
    INTERPOLATION_MODE          _colorsInterpolationMode;
    pxr::VtArray<pxr::GfVec3f>  _colors;

    //std::vector<UsdEmbreeMeshDatas> _extraDatas;
  };

  UsdEmbreeMesh* TranslateMesh( RTCDevice device,
                                RTCScene scene,
                                const pxr::UsdGeomMesh& usdMesh, 
                                double time);

  void CheckNormals(const pxr::UsdGeomMesh& usdMesh,
                    double time,
                    UsdEmbreeMesh* mesh);

  int TriangulateMesh(const pxr::VtArray<int>& counts, 
                      const pxr::VtArray<int>& indices, 
                      pxr::VtArray<int>& triangles,
                      pxr::VtArray<int>& samples);

  template<typename T>
  int TriangulateData(const pxr::VtArray<int>& indices, 
                      const pxr::VtArray<T>& datas,
                      pxr::VtArray<T>& result);

} // namespace embree
