#pragma once

#include "../default.h"

AMN_NAMESPACE_OPEN_SCOPE

void 
ComputeVertexNormals(const pxr::VtArray<pxr::GfVec3f>& positions,
                    const pxr::VtArray<int>& counts,
                    const pxr::VtArray<int>& indices,
                    const pxr::VtArray<int>& triangles,
                    pxr::VtArray<pxr::GfVec3f>& normals);

int 
TriangulateMesh(const pxr::VtArray<int>& counts, 
                const pxr::VtArray<int>& indices, 
                pxr::VtArray<int>& triangles,
                pxr::VtArray<int>& samples);

template<typename T>
void 
TriangulateData(const pxr::VtArray<int>& indices, 
                const pxr::VtArray<T>& datas,
                pxr::VtArray<T>& result)
{
  result.resize(indices.size());
  for(int i=0;i<indices.size();++i)
  {
    result[i] = datas[indices[i]];
  }
};


AMN_NAMESPACE_CLOSE_SCOPE