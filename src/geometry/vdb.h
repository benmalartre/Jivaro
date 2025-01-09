#ifndef JVR_GEOMETRY_VDB_H
#define JVR_GEOMETRY_VDB_H
#pragma once

#include <memory>
#include <iostream>
#include <fstream>
#include <openvdb/openvdb.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "mesh.h"

JVR_NAMESPACE_OPEN_SCOPE

class VDB {
public:
  typedef std::shared_ptr<VDB> Ptr;
  GfMatrix4f _xfo;
  openvdb::FloatGrid::Ptr _grid;
  float _isovalue;
  bool _initialized;

  void LoadMesh(Mesh& toLoad, float resolution = 1.0, int band = 3);
  void LoadVolume(std::ifstream & buf, int w, int h , int d, 
    float resolution = 1.0);
  void LevelSphere(const GfVec3f& center, float radius,
    float voxelSize=0.5f, float width=4.f);
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
  bool IntersectRay(const GfVec3f& pt, const GfVec3f& dir, 
    GfVec3f& out);
  bool IntersectRay(const float x, const float y, const float z, 
    const float dx, const float dy, const float dz, 
    float & ox, float &oy, float &oz, float &t);
  bool IntersectRay(const GfVec3f& pt, const GfVec3f& dir, 
    GfVec3f& out, float &t);
  void Blur();
  void Taubin();

  void Transform(const GfMatrix4f& mat);
  void Translate(const GfVec3f& dir);
  void Rotate(const GfVec3f& axis, float angle);
  GfRange3f GetBBox();
  void Save(std::string filename);

  void ToMesh(Mesh* mesh);

  VDB();
  VDB(Mesh& m, float resolution = 1.0);
  VDB(const VDB & vdb);
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_VDB_H