#ifndef AMN_APPLICATION_SHAPE_H
#define AMN_APPLICATION_SHAPE_H
#pragma once

#include "../common.h"
#include <vector>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/range3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/imaging/garch/glApi.h>

AMN_NAMESPACE_OPEN_SCOPE

class GLSLProgram;
class Shape {
public:
  enum Type {
    GRID,
    BOX,
    SPHERE,
    ICOSPHERE,
    DISC,
    CYLINDER,
    TUBE,
    CONE,
    CAPSULE,
    TORUS
  };

  struct Component {
    short type;
    short index;
    size_t start;
    size_t end;
    bool active;
    bool hovered;
    pxr::GfRange3f bounds;
    pxr::GfMatrix4f offset;
    pxr::GfVec4f color;
    

    Component(short type, short index, size_t start, size_t end, 
      const pxr::GfVec4f& color, const pxr::GfMatrix4f& m) 
      : type(type)
      , index(index)
      , start(start)
      , end(end)
      , color(color)
      , offset(m)
      , active(false)
      , hovered(false){};
    ~Component(){};

    void SetBounds(const pxr::GfVec3f& xyz);
    void ComputeBounds(Shape* shape);
    size_t Intersect(const pxr::GfRay& ray, const pxr::GfMatrix4f& m);
  };

  Shape();
  ~Shape();
  
  size_t GetNumPoints() {return _points.size();};
  size_t GetNumTriangles() {return _indices.size() / 3;};
  size_t GetNumIndices() {return _indices.size();};
  size_t GetNumComponents() {return _components.size();};
  const std::vector<pxr::GfVec3f>& GetPoints() {return _points;};
  const Component& GetComponent(size_t index) {return _components[index];};

  static void _MakeCircle(std::vector<pxr::GfVec3f>& points, float radius, 
    size_t numPoints, const pxr::GfMatrix4f& m= pxr::GfMatrix4f());

  void _TransformPoints(size_t start, size_t end, 
    const pxr::GfMatrix4f& m);

  void UpdateComponents(short hovered, short active);
  void AddComponent(const Component& component);
  void AddComponent(short type, short index, size_t start, size_t end, 
      const pxr::GfVec4f& color, const pxr::GfMatrix4f& m);
  Component AddGrid(size_t index, float width=1.f, float depth=1.f,  size_t divX=8, 
    size_t divZ=8, const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddBox(size_t index, float width=1.f, float height=1.f, float depth=1.f, 
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddSphere(size_t index, float radius, size_t lats=8, size_t longs=8,
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddIcoSphere(size_t index, float radius, size_t subdiv=0, 
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddDisc(size_t index, float radius, size_t lats=8, 
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddDisc(size_t index, float radius, float start, float end, size_t lats=8,
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddCylinder(size_t index, float radius, float height, size_t lats=8, 
    size_t longs=2, const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddTube(size_t index, float outRadius, float inRadius, float height, 
    size_t lats=8, size_t longs=2, const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddCone(size_t index, float radius, float height, size_t lats=8, 
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddTorus(size_t index, float radius, float section, size_t lats=16, 
    size_t longs=8, const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  
  size_t Intersect(const pxr::GfRay& ray, const pxr::GfMatrix4f& m);

  void Clear();

  static void UpdateCamera(const pxr::GfMatrix4f& view, 
    const pxr::GfMatrix4f& proj);

  void Setup();
  void Draw(const pxr::GfMatrix4f& model, const pxr::GfVec4f& color);
  void Draw(const pxr::GfMatrix4f& model, const pxr::GfVec4f& color,
    size_t start, size_t end);

private:
  std::vector<pxr::GfVec3f> _points;
  std::vector<int> _indices;
  std::vector<Component> _components;
  GLuint _vao;
  GLuint _vbo;
  GLuint _eab;
};

// static shapes
extern Shape* GRID_SHAPE;
extern Shape* BOX_SHAPE;
extern Shape* SPHERE_SHAPE;
extern Shape* ICOSPHERE_SHAPE;
extern Shape* DISC_SHAPE;
extern Shape* CYLINDER_SHAPE;
extern Shape* TUBE_SHAPE;
extern Shape* CONE_SHAPE;
extern Shape* CAPSULE_SHAPE;
extern Shape* TORUS_SHAPE;

extern GLSLProgram* SHAPE_PROGRAM;
static bool SHAPE_INITIALIZED = false;


void InitStaticShapes();

AMN_NAMESPACE_CLOSE_SCOPE

#endif