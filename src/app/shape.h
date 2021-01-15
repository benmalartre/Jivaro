#ifndef AMN_APPLICATION_SHAPE_H
#define AMN_APPLICATION_SHAPE_H
#pragma once

#include "../common.h"
#include <vector>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

AMN_NAMESPACE_OPEN_SCOPE

enum SHAPES {
  GRID,
  BOX,
  SPHERE,
  ICOSPHERE,
  CYLINDER,
  TUBE,
  CONE,
  CAPSULE,
  TORUS
};

class Shape {
public:
  Shape();
  ~Shape();
  size_t NumPoints(){return _numPoints;};
  size_t NumTriangles(){return _numTriangles;};

  static void _MakeCircle(std::vector<pxr::GfVec3f>& points, float radius, 
    size_t numPoints, const pxr::GfMatrix4f& m= pxr::GfMatrix4f());

  void _TransformPoints(size_t startIdx, size_t endIdx, 
    const pxr::GfMatrix4f& m);

  void AddGrid(float width=1.f, float depth=1.f,  size_t divX=8, 
    size_t divZ=8, const pxr::GfMatrix4f& m=pxr::GfMatrix4f());
  void AddBox(float width=1.f, float height=1.f, float depth=1.f, 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f());
  void AddSphere(float radius, size_t lats=8, size_t longs=8,
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f());
  void AddIcoSphere(float radius, size_t subdiv=0, 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f());
  void AddCylinder(float radius, float height, size_t lats=8, 
    size_t longs=2, const pxr::GfMatrix4f& m=pxr::GfMatrix4f());
  void AddTube(float outRadius, float inRadius, float height, size_t lats=8,
    size_t longs=2, const pxr::GfMatrix4f& m=pxr::GfMatrix4f());
  void AddCone(float radius, float height, size_t lats=8, 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f());

private:
  size_t _numPoints;
  size_t _numTriangles;
  std::vector<pxr::GfVec3f> _points;
  std::vector<int> _indices;
};

// static shapes
static Shape GRID_SHAPE;
static Shape BOX_SHAPE;
static Shape SPHERE_SHAPE;
static Shape ICOSPHERE_SHAPE;
static Shape CYLINDER_SHAPE;
static Shape TUBE_SHAPE;
static Shape CONE_SHAPE;
static Shape CAPSULE_SHAPE;
static Shape TORUS_SHAPE;

static bool SHAPE_INITIALIZED = false;

void InitStaticShapes();

//AddCylinder(&CYLINDER_SHAPE, 0.05f, 1.f, 8, 1, pxr::GfMatrix4f());

AMN_NAMESPACE_CLOSE_SCOPE

#endif