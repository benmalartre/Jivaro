//
// Copyright 2020 benmalartre
//
// unlicensed
//
#ifndef PXR_USD_IMAGING_USD_NPR_IMAGING_HALFEDGE_H
#define PXR_USD_IMAGING_USD_NPR_IMAGING_HALFEDGE_H

#include "pxr/pxr.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/hashmap.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec4f.h"
#include "pxr/base/gf/vec4d.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/imaging/hd/types.h"
#include "pxr/imaging/hd/changeTracker.h"
#include "pxr/imaging/hd/meshTopology.h"

#include <vector>
#include <memory>


PXR_NAMESPACE_OPEN_SCOPE

enum UsdHalfEdgeFlags {
  EDGE_BOUNDARY = 1,
  EDGE_CREASE = 2,
  EDGE_SILHOUETTE = 4
};

enum UsdHalfEdgeMeshVaryingBits {
  VARYING_TOPOLOGY = 1,
  VARYING_DEFORM = 2,
  VARYING_TRANSFORM = 4,
  VARYING_VISIBILITY = 8
};

struct UsdNprHalfEdge
{
  uint32_t                vertex;    // vertex index
  uint32_t                triangle;  // triangle index
  struct UsdNprHalfEdge*  twin;      // opposite half-edge
  struct UsdNprHalfEdge*  next;      // next half-edge

  UsdNprHalfEdge():vertex(0),twin(NULL),next(NULL){};
  void GetTriangleNormal(const GfVec3f* positions, GfVec3f& normal) const;
  bool GetFacing(const GfVec3f* positions, const GfVec3f& v) const;
  bool GetFacing(const GfVec3f* positions, const GfVec3f* normals,
    const GfVec3f& v) const;
  short GetFlags(const GfVec3f* positions, const GfVec3f* normals, 
    const GfVec3f& v, float creaseValue) const;
};

/// \class UsdNprHalfEdgeMesh
///
class UsdNprHalfEdgeMesh
{
public:
  UsdNprHalfEdgeMesh(const SdfPath& path, char varyingBits);
  void Init(const UsdGeomMesh& mesh, const UsdTimeCode& timeCode);
  void Update(const UsdGeomMesh& mesh, const UsdTimeCode& timeCode);
  const std::vector<UsdNprHalfEdge>& GetHalfEdges() const {return _halfEdges;};

  const GfVec3f* GetPositionsPtr() const {return &_positions[0];};
  const GfVec3f* GetNormalsPtr() const {return &_normals[0];};
  size_t GetNumPoints() const {return _positions.size();};
  size_t GetNumTriangles() const {return _numTriangles;};
  size_t GetNumHalfEdges() const {return _halfEdges.size();};

  // object
  const SdfPath& GetPath(){return _sdfPath;};

  // xform
  void SetMatrix(const GfMatrix4d& m){_xform = GfMatrix4f(m);};

  // varying
  bool IsVarying() const {return _varyingBits != 0;};
  bool IsTopoVarying() const {return _varyingBits & VARYING_TOPOLOGY;};
  bool IsDeformVarying() const {return _varyingBits & VARYING_DEFORM;};
  bool IsTransformVarying() const {return _varyingBits & VARYING_TRANSFORM;};
  bool IsVisibilityVarying() const {return _varyingBits & VARYING_VISIBILITY;};
  char GetVaryingBits() const {return _varyingBits;};

  // silhouettes
  void FindSilhouettes(const GfMatrix4d& viewMatrix, 
    std::vector<const UsdNprHalfEdge*>& silhouettes);

  // output
  void ComputeOutputGeometry(std::vector<const UsdNprHalfEdge*>& silhouettes,
    const GfVec3f& viewPoint, VtArray<GfVec3f>& points, 
    VtArray<int>& faceVertexCounts, VtArray<int>& faceVertexIndices);

  // time
  void SetLastTime(const UsdTimeCode& timeCode){_lastTime = timeCode;};
  const UsdTimeCode& GetLastTime(){return _lastTime;};

  // mutex
  std::mutex& GetMutex(){return _mutex;};

private:
  SdfPath                     _sdfPath;
  GfMatrix4f                  _xform;
  std::vector<UsdNprHalfEdge> _halfEdges; 
  VtArray<int>                _samples;
  VtArray<GfVec3f>            _positions;
  VtArray<GfVec3f>            _normals;
  size_t                      _numTriangles;
  char                        _varyingBits;
  UsdTimeCode                 _lastTime;
  mutable std::mutex                  _mutex;

};

typedef std::shared_ptr<UsdNprHalfEdgeMesh> UsdNprHalfEdgeMeshSharedPtr;

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_USD_IMAGING_USD_NPR_IMAGING_HALFEDGE_H