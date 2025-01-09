//======================================================
// TRIANGLE DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_TRIANGLE_H
#define JVR_GEOMETRY_TRIANGLE_H

#include "../geometry/component.h"

JVR_NAMESPACE_OPEN_SCOPE

#define FINDMINMAX(x0,x1,x2,min,max)  \
  min = max = x0;                     \
  if(x1<min) min=x1;                  \
  if(x1>max) max=x1;                  \
  if(x2<min) min=x2;                  \
  if(x2>max) max=x2;


// ======================== X-tests ========================
#define AXISTEST_X01(a, b, fa, fb)                    \
  p0 = a*v0[1] - b*v0[2];                             \
  p2 = a*v2[1] - b*v2[2];                             \
  if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}  \
  rad = fa * boxhalfsize[1] + fb * boxhalfsize[2];    \
  if(min>rad || max<-rad) return 0; 

#define AXISTEST_X2(a, b, fa, fb)                     \
  p0 = a*v0[1] - b*v0[2];                             \
  p1 = a*v1[1] - b*v1[2];                             \
  if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}  \
  rad = fa * boxhalfsize[1] + fb * boxhalfsize[2];    \
  if(min>rad || max<-rad) return 0;


// ======================== Y-tests ========================
#define AXISTEST_Y02(a, b, fa, fb)                    \
  p0 = -a*v0[0] + b*v0[2];                            \
  p2 = -a*v2[0] + b*v2[2];                            \
  if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}  \
  rad = fa * boxhalfsize[0] + fb * boxhalfsize[2];    \
  if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)                     \
  p0 = -a*v0[0] + b*v0[2];                            \
  p1 = -a*v1[0] + b*v1[2];                            \
  if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}  \
  rad = fa * boxhalfsize[0] + fb * boxhalfsize[2];    \
  if(min>rad || max<-rad) return 0;


// ======================== Z-tests ========================
#define AXISTEST_Z12(a, b, fa, fb)                    \
  p1 = a*v1[0] - b*v1[1];                             \
  p2 = a*v2[0] - b*v2[1];                             \
  if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}  \
  rad = fa * boxhalfsize[0] + fb * boxhalfsize[1];    \
  if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)                     \
  p0 = a*v0[0] - b*v0[1];                             \
  p1 = a*v1[0] - b*v1[1];                             \
  if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}  \
  rad = fa * boxhalfsize[0] + fb * boxhalfsize[1];    \
  if(min>rad || max<-rad) return 0;


class Mesh;
class Location;
struct Triangle : public Component{

  Triangle()
    : Component() {};
  Triangle(uint32_t index, const GfVec3i& vertices)
    : Component(index)
    , vertices(vertices) {};

  GfVec3i vertices;

  GfVec3f GetCenter(const GfVec3f* points) const;
  GfVec3f GetNormal(const GfVec3f* points) const;
  GfVec3f GetVelocity(const GfVec3f* points, const GfVec3f* previous) const;
  float GetArea(const GfVec3f* points) const;
  bool PlaneBoxTest(const GfVec3f& normal, const GfVec3f& point, 
    const GfVec3f& box) const;

  // overrides
  virtual bool Raycast(const GfVec3f* points, const GfRay& ray, Location* hit) const override;
  virtual bool Closest(const GfVec3f* points, const GfVec3f& point, Location* hit) const override;
  virtual bool Touch(const GfVec3f* points, const GfVec3f& center, const GfVec3f& boxhalfsize) const override;

  virtual GfRange3f GetBoundingBox(const GfVec3f* positions, const GfMatrix4d& m) const override;
  virtual short GetType() const override {return Component::TRIANGLE;};

};

struct TrianglePair : public Component {
  Triangle* left;
  Triangle* right;

  TrianglePair(uint32_t index, Triangle* t1, Triangle* t2)
    : Component(index)
    , left(t1)
    , right(t2) {};

  GfVec4i GetVertices() const;

  virtual bool Raycast(const GfVec3f* points, const GfRay& ray, Location* hit) const override;
  virtual bool Closest(const GfVec3f* points, const GfVec3f& point, Location* hit) const override;
  virtual bool Touch(const GfVec3f* points, const GfVec3f& center, const GfVec3f& halfSize) const override;

  virtual GfRange3f GetBoundingBox(const GfVec3f* positions, const GfMatrix4d& m) const override;
  virtual short GetType() const override {return Component::TRIANGLEPAIR;};
};

/// Longest edge in a triangle
static size_t GetLongestEdgeInTriangle(const GfVec3i& vertices, 
  const GfVec3f* positions);

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_TRIANGLE_H