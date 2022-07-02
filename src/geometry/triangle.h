//======================================================
// TRIANGLE DECLARATION
//======================================================
#ifndef JVR_GEOMETRY_TRIANGLE_H
#define JVR_GEOMETRY_TRIANGLE_H

#include "../common.h"
#include <pxr/base/gf/vec3f.h>
#include <algorithm>
#include <math.h>
#include <stdio.h>

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
struct Triangle {
  uint32_t     id;
  pxr::GfVec3i vertices;

  void GetCenter(Mesh* mesh, pxr::GfVec3f& center);
  void GetNormal(Mesh* mesh, pxr::GfVec3f& normal);
  float GetArea(Mesh* mesh);
  void ClosestPoint(Mesh* mesh, const pxr::GfVec3f& point , 
    pxr::GfVec3f& closest, float& u, float& v, float& w);
  bool Touch(Mesh* mesh, const pxr::GfVec3f& center, 
    const pxr::GfVec3f& boxhalfsize);
  bool PlaneBoxTest(const pxr::GfVec3f& normal, const pxr::GfVec3f& point, 
    const pxr::GfVec3f& box);

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GEOMETRY_TRIANGLE_H