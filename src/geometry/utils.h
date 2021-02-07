#ifndef AMN_GEOMETRY_UTILS_H
#define AMN_GEOMETRY_UTILS_H

#include "../common.h"
#include "triangle.h"
#include <iostream>
#include "pxr/pxr.h"

#include "pxr/base/vt/array.h"
#include "pxr/base/gf/vec3i.h"
#include "pxr/base/gf/vec3f.h"

AMN_NAMESPACE_OPEN_SCOPE

/// Triangulate a polygonal mesh
int 
TriangulateMesh(const pxr::VtArray<int>& counts, 
                const pxr::VtArray<int>& indices, 
                pxr::VtArray<Triangle>& triangles);

/// Compute smooth vertex normals on a triangulated polymesh
void 
ComputeVertexNormals(const pxr::VtArray<pxr::GfVec3f>& positions,
                     const pxr::VtArray<int>& counts,
                     const pxr::VtArray<int>& indices,
                     const pxr::VtArray<Triangle>& triangles,
                     pxr::VtArray<pxr::GfVec3f>& normals);

/// Compute triangle normals
void 
ComputeTriangleNormals( const pxr::VtArray<pxr::GfVec3f>& positions,
                        const pxr::VtArray<Triangle>& triangles,
                        pxr::VtArray<pxr::GfVec3f>& normals);
                          
/// Triangulate data
/// No checks are made on data type or array bounds
/// 
template<typename DataType>
void
TriangulateDatas( const pxr::VtArray<Triangle>& triangles,
                  const pxr::VtArray<DataType>& datas,
                  pxr::VtArray<DataType>& result)
{
  size_t numTriangles = triangles.size();
  result.resize(numTriangles * 3);
  for(int i = 0; i < numTriangles; ++i) {
    const Triangle* T = &triangles[i];
    result[i * 3    ] = datas[T->vertices[0]];
    result[i * 3 + 1] = datas[T->vertices[1]];
    result[i * 3 + 2] = datas[T->vertices[2]];
  }
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif