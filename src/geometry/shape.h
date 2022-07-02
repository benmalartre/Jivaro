#ifndef JVR_APPLICATION_SHAPE_H
#define JVR_APPLICATION_SHAPE_H

#include "../common.h"
#include <vector>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/range3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/imaging/garch/glApi.h>

JVR_NAMESPACE_OPEN_SCOPE

class GLSLProgram;
class Shape {
public:
  enum Flag {
    SELECTED  = 1,
    HOVERED   = 2,
    VISIBLE   = 4,
    PICKABLE  = 8,
    HELPER    = 16,
    FLAT      = 32,
    MASK      = 64
  };

  enum Type {
    GRID,
    BOX,
    SPHERE,
    ICOSPHERE,
    DISC,
    RING,
    CYLINDER,
    TUBE,
    CONE,
    CAPSULE,
    TORUS,
    EXTRUSION
  };

  enum Usage {
    STATIC,
    DYNAMIC
  };

  struct Component {
    typedef short (Shape::Component::*IntersectFunc)(const pxr::GfRay& ray, 
      const pxr::GfMatrix4f& m, double* distance, double scale);

    short               flags;
    short               type;
    int                 index;
    size_t              basePoints;
    size_t              numPoints;
    size_t              baseIndices;
    size_t              numIndices;
    pxr::GfRange3f      bounds;
    pxr::GfMatrix4f     offsetMatrix;
    pxr::GfMatrix4f     parentMatrix;
    pxr::GfVec4f        color;

    IntersectFunc _intersectImplementation;
    
    Component(short type, short shapeIndex, size_t basePointIndex, 
      size_t numPoints, size_t baseIndices, size_t numIndices, 
      const pxr::GfVec4f& color, const pxr::GfMatrix4f& parentMatrix, 
      const pxr::GfMatrix4f& offsetMatrix=pxr::GfMatrix4f(1.f)) 
      : flags(VISIBLE|PICKABLE)
      , type(type)
      , index(shapeIndex)
      , basePoints(basePointIndex)
      , numPoints(numPoints)
      , baseIndices(baseIndices)
      , numIndices(numIndices)
      , color(color)
      , parentMatrix(parentMatrix)
      , offsetMatrix(offsetMatrix) {
        switch(type) {
          case GRID:
            _intersectImplementation = &Shape::Component::_IntersectGrid;
            break;
          case BOX:
            _intersectImplementation = &Shape::Component::_IntersectBox;
            break;
          case SPHERE:
          case ICOSPHERE:
            _intersectImplementation = &Shape::Component::_IntersectSphere;
            break;
          case DISC:
            _intersectImplementation = &Shape::Component::_IntersectDisc;
            break;
          case RING:
            _intersectImplementation = &Shape::Component::_IntersectRing;
            break;
          case TUBE:
            _intersectImplementation = &Shape::Component::_IntersectTube;
            break;
          case CYLINDER:
            _intersectImplementation = &Shape::Component::_IntersectCylinder;
            break;
          case TORUS:
              _intersectImplementation = &Shape::Component::_IntersectTorus;
            break;
          default:
            _intersectImplementation = 0;
        }
      };
    ~Component(){};

    void SetBounds(const pxr::GfVec3f& xyz);
    void SetBounds(const pxr::GfRange3f&);
    void SetFlag(short flag, bool value);
    bool GetFlag(short flag) const;
    void ComputeBounds(Shape* shape);

    short _IntersectGrid(const pxr::GfRay& ray, const pxr::GfMatrix4f& m, double* distance, double scale=1.0);
    short _IntersectBox(const pxr::GfRay& ray, const pxr::GfMatrix4f& m, double* distance, double scale = 1.0);
    short _IntersectSphere(const pxr::GfRay& ray, const pxr::GfMatrix4f& m, double* distance, double scale = 1.0);
    short _IntersectDisc(const pxr::GfRay& ray, const pxr::GfMatrix4f& m, double* distance, double scale = 1.0);
    short _IntersectRing(const pxr::GfRay& ray, const pxr::GfMatrix4f& m, double* distance, double scale = 1.0);
    short _IntersectCylinder(const pxr::GfRay& ray, const pxr::GfMatrix4f& m, double* distance, double scale = 1.0);
    short _IntersectTube(const pxr::GfRay& ray, const pxr::GfMatrix4f& m, double* distance, double scale = 1.0);
    short _IntersectTorus(const pxr::GfRay&, const pxr::GfMatrix4f& m, double* distance, double scale = 1.0);

    short Intersect(const pxr::GfRay& ray, const pxr::GfMatrix4f& m, double* distance, double scale = 1.0);
  };

  Shape(short usage=STATIC);
  ~Shape();
  
  size_t GetNumPoints() {return _points.size();};
  size_t GetNumTriangles() {return _indices.size() / 3;};
  size_t GetNumIndices() {return _indices.size();};
  size_t GetNumComponents() {return _components.size();};
  const std::vector<pxr::GfVec3f>& GetPoints() {return _points;};
  const std::vector<int>& GetIndices() {return _indices;};
  const Component& GetComponent(size_t index) const {return _components[index];};
  Component& GetComponent(size_t index) { return _components[index]; };

  static void _MakeCircle(std::vector<pxr::GfVec3f>& points, float radius, 
    size_t numPoints, const pxr::GfMatrix4f& m= pxr::GfMatrix4f());

  void _TransformPoints(size_t start, size_t end, 
    const pxr::GfMatrix4f& m);
  pxr::GfVec3f _GetComponentAxis(const Shape::Component& component,
    const pxr::GfMatrix4f& m);

  void UpdateComponents(short hovered, short active, bool hideInactive=false);
  void UpdateVisibility(const pxr::GfMatrix4f& m, const pxr::GfVec3f& dir);
  void AddComponent(const Component& component);
  void RemoveComponent(size_t idx);
  void RemoveLastComponent();
  /*void AddComponent(short type, short index, size_t basePoint, size_t numPoints,
    size_t startIndex, size_t endIndex, const pxr::GfVec4f& color, 
    const pxr::GfMatrix4f& m);*/
  Component AddGrid(short index, float width=1.f, float depth=1.f,  size_t divX=8, 
    size_t divZ=8, const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddBox(short index, float width=1.f, float height=1.f, float depth=1.f, 
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddSphere(short index, float radius, size_t lats=8, size_t longs=8,
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddIcoSphere(short index, float radius, size_t subdiv=0, 
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddDisc(short index, float radius, size_t lats=8, 
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddDisc(short index, float radius, float start, float end, size_t lats=8,
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddRing(short index, float radius, float section, size_t lats=16,
    const pxr::GfVec4f& color = pxr::GfVec4f(1.f),
    const pxr::GfMatrix4f& m = pxr::GfMatrix4f(1.f));
  Component AddCylinder(short index, float radius, float height, size_t lats=8, 
    size_t longs=2, const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddTube(short index, float outRadius, float inRadius, float height, 
    size_t lats=8, size_t longs=2, const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddCone(short index, float radius, float height, size_t lats=8, 
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddTorus(short index, float radius, float section, size_t lats=16, 
    size_t longs=8, const pxr::GfVec4f& color=pxr::GfVec4f(1.f), 
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));
  Component AddExtrusion(short index, 
    const std::vector<pxr::GfMatrix4f>& xfos,
    const std::vector<pxr::GfVec3f>& profile, 
    const pxr::GfVec4f& color=pxr::GfVec4f(1.f),
    const pxr::GfMatrix4f& m=pxr::GfMatrix4f(1.f));

  void SetVisibility(int bits);
  short Intersect(const pxr::GfRay& ray, const pxr::GfMatrix4f& m, const pxr::GfMatrix4f& v);

  void Clear();

  static void UpdateCamera(const pxr::GfMatrix4f& view, 
    const pxr::GfMatrix4f& proj);

  void Setup(bool dynamic=false);
  void Draw(const pxr::GfMatrix4f& model, const pxr::GfVec4f& color);
  void Draw(const pxr::GfMatrix4f& model, const pxr::GfVec4f& color,
    size_t start, size_t end);

  void Bind(GLSLProgram* pgm);
  void Unbind();
  void DrawComponent(size_t index, const pxr::GfMatrix4f& model, 
    const pxr::GfVec4f& color);

private:
  std::vector<pxr::GfVec3f> _points;
  std::vector<int> _indices;
  std::vector<Component> _components;
  short _usage;
  float _scale;
  GLuint _vao;
  GLuint _vbo;
  GLuint _eab;
  GLuint _uModel;
  GLuint _uColor;
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


void InitShapeShader();

JVR_NAMESPACE_CLOSE_SCOPE

#endif