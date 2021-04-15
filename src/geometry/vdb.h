#ifndef AMN_GEOMETRY_VDB_H
#define AMN_GEOMETRY_VDB_H
#pragma once

#include <memory>
#include <iostream>
#include <fstream>
#include <openvdb/openvdb.h>
#include <pxr/base/gf/matrix4f.h>
#include "mesh.h"


AMN_NAMESPACE_OPEN_SCOPE

class VDB {
public:
  typedef std::shared_ptr<VDB> Ptr;
  pxr::GfMatrix4f _xfo;
  openvdb::FloatGrid::Ptr _grid;
  Mesh _mesh;
  float _isovalue;
  bool _initialized;

  void LoadMesh(Mesh& toLoad, float resolution = 1.0, int band = 3);
  void LoadVolume(std::ifstream & buf, int w, int h , int d, 
    float resolution = 1.0);
  void Clear();
  void Offset(float amt);
  void Load(std::string filename);
  void DoUnion(VDB & vdb);
  void DoDifference(VDB & vdb);
  void DoUnion(openvdb::FloatGrid::Ptr vdb);
  void DoIntersect(VDB & vdb);
  void SetThreshold(float thresh);

  void FloodFill();

  bool IntersectRay(const float x, const float y, const float z, 
    const float dx, const float dy, const float dz, 
    float & ox, float &oy, float &oz);
  bool IntersectRay(const pxr::GfVec3f& pt, const pxr::GfVec3f& dir, 
    pxr::GfVec3f& out);
  bool IntersectRay(const float x, const float y, const float z, 
    const float dx, const float dy, const float dz, 
    float & ox, float &oy, float &oz, float &t);
  bool IntersectRay(const pxr::GfVec3f& pt, const pxr::GfVec3f& dir, 
    pxr::GfVec3f& out, float &t);
  void Blur();
  void Taubin();

  void UpdateMesh();

  void Transform(const pxr::GfMatrix4f& mat);
  void Translate(const pxr::GfVec3f& dir);
  void Rotate(const pxr::GfVec3f& axis, float angle);
  pxr::GfRange3f GetBBox();
  void Save(std::string filename);

  Mesh ToMesh();

  VDB();
  VDB(Mesh& m, float resolution = 1.0);
  VDB(const VDB & vdb);
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_GEOMETRY_VDB_H