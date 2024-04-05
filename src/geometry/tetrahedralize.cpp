#include "../geometry/tetrahedralize.h"

JVR_NAMESPACE_OPEN_SCOPE

static pxr::GfVec3d _Dirs[6] = {
  { 1.0, 0.0, 0.0 },
  { 0.0,-1.0, 0.0 },
  { 0.0, 1.0, 0.0 },
  { 0.0,-1.0, 0.0 },
  { 0.0, 0.0, 1.0 },
  { 0.0, 0.0,-1.0 }
};

static pxr::GfVec3i _Faces[4] = {
  {2, 1, 0}, 
  {0, 1, 3}, 
  {1, 2, 3}, 
  {2, 0, 3}
};


bool Tetrahedralize::_IsInside(const pxr::GfVec3f& p, double minDist = 0.0)
{
  size_t numIn = 0;
  Location hit;
  const pxr::GfVec3f* _geometry->GetPositionsCPtr();
  for(size_t i = 0; i < 6; ++i) {
    if(_bvh.Raycast(points, pxr::GfRay(p, _Dirs[i]), 
      &hit, FLT_MAX, &minDist)) {
        const pxr::GfVec3f normal = hit.GetNormal(_geometry);
        std::cout << "hit normal : " << normal << std::endl;
        if(pxr::GfDot(normal, _Dirs[i]) > 0.0) numIn++;
        if(minDist > 0.0 && hit.GetT() < minDist)return false;
    }
  }
  std::cout << "is inside : " << (numIn > 3) << std::endl;
  return numIn > 3;
}

pxr::GfVec3f Tetrahedralize::_GetCircumCenter(
  const pxr::GfVec3f& p0, const pxr::GfVec3f& p1, 
  const pxr::GfVec3f& p2, const pxr::GfVec3f& p3)
{
  const pxr::GfVec3f b = p1 - p0;
  const pxr::GfVec3f c = p2 - p0;
  const pxr::GfVec3f d = p3 - p0;

  float det = 2.0 * (
    b[0]*(c[1]*d[2] - c[2]*d[1]) - 
    b[1]*(c[0]*d[2] - c[2]*d[0]) + 
    b[2]*(c[0]*d[1] - c[1]*d[0])
  );

  if(pxr::GfIsClose(det, 0.f, 1-e9)) {
    return p0;
  } else {
    pxr::GfVec3f v = 
      (c ^ d) * pxr::GfDot(b, b) + 
      (d ^ b) * pxr::GfDot(c, c) + 
      (b ^ c) * pxr::GfDot(d, d);
    v *= 1.f / det;
    return p0 + v;
  }
}


float Tetrahedralize::_TetQuality(
  const pxr::GfVec3f& p0, const pxr::GfVec3f& p1, 
  const pxr::GfVec3f& p2, const pxr::GfVec3f& p3)
{
  const pxr::GfVec3f d0 = p1 - p0;
  const pxr::GfVec3f d1 = p2 - p0;
  const pxr::GfVec3f d2 = p3 - p0;
  const pxr::GfVec3f d3 = p2 - p1;
  const pxr::GfVec3f d4 = p3 - p2;
  const pxr::GfVec3f d5 = p1 - p3;

  const double s0 = d0.GetLength();
  const double s1 = d1.GetLength();
  const double s2 = d2.GetLength();
  const double s3 = d3.GetLength();
  const double s4 = d4.GetLength();
  const double s5 = d5.GetLength();

  const double ms = (s0*s0 + s1*s1 + s2*s2 + s3*s3 + s4*s4 + s5*s5) / 6.0;
  const double rms = pxr::GfSqrt(ms);

  const double s = 12.0 / pxr::GfSqrt(2.0);

  const vol = pxr::GfDot(d0, d1 ^ d2) / 6.0;
  return s * vol / (rms * rms * rms);
  // 1.0 for regular tetrahedron
}
    

bool Tetrahedralize::_CompareEdges(e0, e1)
{
  if(e0[0] < e1[0] || (e0[0] == e1[0] && e0[1] < e1[1])) {
    return -1;
  } else {
    return 1;
  }
}

bool Tetrahedralize::_EqualEdges(e0, e1)
{
  return(e0[0] == e1[0] && e0[1] == e1[1]);
}

JVR_NAMESPACE_CLOSE_SCOPE
