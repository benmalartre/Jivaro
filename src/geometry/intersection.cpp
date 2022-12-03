#include <pxr/base/gf/math.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/plane.h>

#include "../geometry/intersection.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"

JVR_NAMESPACE_OPEN_SCOPE

//=================================================================================================
// HIT CLASS
//=================================================================================================
void 
Hit::Set(const Hit& other) {
  _geom = other._geom;
  _baryCoords = other._baryCoords;
  _elemType = other._elemType;
  _elemId = other._elemId;
  _t = other._t;
}

void 
Hit::GetPosition(pxr::GfVec3f* position) const 
{
  /*
  Geometry*     _geom;
  pxr::GfVec3f  _baryCoords;
  short         _elemType;
  int           _elemId;
  */
  switch (_geom->GetType()) {
    case Geometry::MESH:
    {
      Mesh* mesh = (Mesh*)_geom;
      Triangle* triangle = mesh->GetTriangle(_elemId);
      const pxr::GfVec3f* positions = mesh->GetPositionsCPtr();
      *position = 
        pxr::GfVec3f(positions[triangle->vertices[0]]) * _baryCoords[0] +
        pxr::GfVec3f(positions[triangle->vertices[1]]) * _baryCoords[1] +
        pxr::GfVec3f(positions[triangle->vertices[2]]) * _baryCoords[2];
      return;
    }
    case Geometry::CURVE:
    {

    }
    case Geometry::POINT:
    {
      Points* points = (Points*)_geom;
      Point point = points->Get(_elemId);
      
      //Point*
    }
  }
}

void 
Hit::GetPosition(const pxr::GfRay& ray, pxr::GfVec3f* position) const
{
  *position = pxr::GfVec3f(ray.GetPoint(_t));
}

void 
Hit::GetNormal(pxr::GfVec3f* normal) const
{
  switch (_geom->GetType()) {
    case Geometry::MESH:
    {

    }
    case Geometry::CURVE:
    {

    }
    case Geometry::POINT:
    {

    }
  }
}

//=================================================================================================
// INTERSECTION ROUTINES
//=================================================================================================
bool 
IntersectDisc(const pxr::GfRay& localRay, const double radius, double* distance) 
{ 
  double hitDistance;
  if(localRay.Intersect(DEFAULT_PLANE, &hitDistance)) {
    pxr::GfVec3d hitPoint = localRay.GetPoint(hitDistance);
    if (hitPoint.GetLength() <= radius) {
      *distance = hitDistance;
      return true;
    }
  }
  return false;
} 

bool 
IntersectRing(const pxr::GfRay& localRay, const double radius, 
  const double section, double* distance)
{
  double hitDistance;
  if (localRay.Intersect(DEFAULT_PLANE, &hitDistance)) {
    pxr::GfVec3d hitPoint = localRay.GetPoint(hitDistance);
    double distanceFromCenter = hitPoint.GetLength();
    if (distanceFromCenter >= (radius - section) && distanceFromCenter <= (radius + section)) {
      *distance = hitDistance;
      return true;
    }
  }
  return false;
}

bool 
IntersectCylinder( const pxr::GfRay& localRay, const double radius, 
  const double height, double* distance)
{
  double enterDistance, exitDistance;
  const pxr::GfVec3d axis(0, 1, 0);
  if(localRay.Intersect(pxr::GfVec3d(0.f), axis,
    radius, &enterDistance, &exitDistance)) {
      // check enter point
      pxr::GfVec3d hit = localRay.GetPoint(enterDistance);
      pxr::GfVec3d projected = hit.GetProjection(axis);
      if (hit[1] >= 0.f && hit[1] <= height) {
        *distance = enterDistance;
        return true;
      }
      // check exit point
      hit = localRay.GetPoint(exitDistance);
      projected = hit.GetProjection(axis);
      if (hit[1] >= 0.f && hit[1] <= height) {
        *distance = exitDistance;
        return true;
      }
  }
  return false;
}

bool 
IntersectTube(const pxr::GfRay& localRay, const double innerRadius,
  const double outerRadius, const double height, double* distance)
{
  // closest point on circle
  double hitDistance;
  double radius = (innerRadius + outerRadius) * 0.5;
  double section = (outerRadius - innerRadius) * 0.5;
  if (localRay.Intersect(DEFAULT_PLANE, &hitDistance)) {
    pxr::GfVec3d hitPoint = localRay.GetPoint(hitDistance);
    double hitLength = hitPoint.GetLength();
    if (hitLength > innerRadius && hitLength < outerRadius) {
      *distance = hitDistance - section;
      return true;
    }
    hitPoint.Normalize();
    // check tangent plane
    pxr::GfVec3d pointOnCircle(hitPoint * radius);
    pxr::GfPlane tangentPlane(
      hitPoint,
      pointOnCircle
    );
    if (localRay.Intersect(tangentPlane, &hitDistance)) {
      hitPoint = localRay.GetPoint(hitDistance);
      if ((hitPoint - pointOnCircle).GetLengthSq() < height * height) {
        *distance = hitDistance;
        return true;
      }
    }
    // check bi-tangent plane
    pxr::GfPlane biTangentPlane(
      hitPoint ^ DEFAULT_PLANE.GetNormal(),
      pointOnCircle
    );
    if (localRay.Intersect(biTangentPlane, &hitDistance)) {
      hitPoint = localRay.GetPoint(hitDistance);
      if ((hitPoint - pointOnCircle).GetLengthSq() < section * section) {
        *distance = hitDistance;
        return true;
      }
    }
  }
  return false;
}

bool 
IntersectTorus( const pxr::GfRay& localRay, const double radius, 
  const double section, double* distance)
{
  
  double po = 1.0;

  const pxr::GfVec3d ro(localRay.GetPoint(0));
  const pxr::GfVec3d rd((localRay.GetPoint(1) - ro).GetNormalized());
  double Ra2 = radius * radius;
  double ra2 = section * section;

  double m = pxr::GfDot(ro, ro);
  double n = pxr::GfDot(ro, rd);

  // bounding sphere
  {
    double h = n * n - m + (radius + section) * (radius + section);
    if (h < 0.0) return false;
  }

  double k = (m + Ra2 - ra2) / 2.0;
  double k3 = n;
  const pxr::GfVec2d rdxz(rd[0], rd[2]);
  const pxr::GfVec2d roxz(ro[0], ro[2]);
  double k2 = n * n - Ra2 * pxr::GfDot(rdxz, rdxz) + k;
  double k1 = n * k - Ra2 * pxr::GfDot(rdxz, roxz);
  double k0 = k * k - Ra2 * pxr::GfDot(roxz, roxz);

  if (pxr::GfAbs(k3 * (k3 * k3 - k2) + k1) < 0.01)
  {
    po = -1.0;
    double tmp = k1; k1 = k3; k3 = tmp;
    k0 = 1.0 / k0;
    k1 = k1 * k0;
    k2 = k2 * k0;
    k3 = k3 * k0;
  }

  double c2 = k2 * 2.0 - 3.0 * k3 * k3;
  double c1 = k3 * (k3 * k3 - k2) + k1;
  double c0 = k3 * (k3 * (c2 + 2.0 * k2) - 8.0 * k1) + 4.0 * k0;
  c2 /= 3.0;
  c1 *= 2.0;
  c0 /= 3.0;
  double Q = c2 * c2 + c0;
  double R = c2 * c2 * c2 - 3.0 * c2 * c0 + c1 * c1;
  double h = R * R - Q * Q * Q;

  if (h >= 0.0)
  {
    h = pxr::GfSqrt(h);
    double v = pxr::GfSgn(R + h) * pxr::GfPow(pxr::GfAbs(R + h), 1.0 / 3.0); // cube root
    double u = pxr::GfSgn(R - h) * pxr::GfPow(pxr::GfAbs(R - h), 1.0 / 3.0); // cube root
    pxr::GfVec2d s((v + u) + 4.0 * c2, (v - u) * sqrt(3.0));
    double z = pxr::GfSqrt(0.5 * (s.GetLength() + s[0]));
    double x = 0.5 * s[1] / z;
    double r = 2.0 * c1 / (x * x + z * z);
    double t1 = x - r - k3; t1 = (po < 0.0) ? 2.0 / t1 : t1;
    double t2 = -x - r - k3; t2 = (po < 0.0) ? 2.0 / t2 : t2;
    double t = 1e20;
    if (t1 > 0.0) t = t1;
    if (t2 > 0.0) t = pxr::GfMin(t, t2);
    *distance = t;
    return true;
  }

  double sQ = pxr::GfSqrt(Q);
  double w = sQ * pxr::GfCos(acos(-R / (sQ * Q)) / 3.0);
  double d2 = -(w + c2); if (d2 < 0.0) return false;
  double d1 = pxr::GfSqrt(d2);
  double h1 = pxr::GfSqrt(w - 2.0 * c2 + c1 / d1);
  double h2 = pxr::GfSqrt(w - 2.0 * c2 - c1 / d1);
  double t1 = -d1 - h1 - k3; t1 = (po < 0.0) ? 2.0 / t1 : t1;
  double t2 = -d1 + h1 - k3; t2 = (po < 0.0) ? 2.0 / t2 : t2;
  double t3 = d1 - h2 - k3; t3 = (po < 0.0) ? 2.0 / t3 : t3;
  double t4 = d1 + h2 - k3; t4 = (po < 0.0) ? 2.0 / t4 : t4;
  double t = 1e20;
  if (t1 > 0.0) t = t1;
  if (t2 > 0.0) t = pxr::GfMin(t, t2);
  if (t3 > 0.0) t = pxr::GfMin(t, t3);
  if (t4 > 0.0) t = pxr::GfMin(t, t4);
  *distance = t;
  return true;
}

bool 
IntersectTorusApprox(const pxr::GfRay& localRay, const double radius,
  const double section, double* distance)
{
  const pxr::GfVec3d ro(localRay.GetPoint(0));
  const pxr::GfVec3d rd((localRay.GetPoint(1) - ro).GetNormalized());
  double Ra2 = radius * radius;
  double ra2 = section * section;

  double m = pxr::GfDot(ro, ro);
  double n = pxr::GfDot(ro, rd);

  // bounding sphere
  {
    double h = n * n - m + (radius + section) * (radius + section);
    if (h < 0.0) return false;
  }

  // closest point on circle
  double hitDistance;
  if (localRay.Intersect(DEFAULT_PLANE, &hitDistance)) {
    pxr::GfVec3d hitPoint = localRay.GetPoint(hitDistance);
    pxr::GfVec3d pointOnCircle(hitPoint.GetNormalized() * radius);
    if ((hitPoint - pointOnCircle).GetLengthSq() < ra2) {
      *distance = hitDistance - section;
      return true;
    }
    else {
      pxr::GfPlane hitCrossPlane(
        hitPoint.GetNormalized() ^ DEFAULT_PLANE.GetNormal(),
        pointOnCircle
      );
      if (localRay.Intersect(hitCrossPlane, &hitDistance)) {
        hitPoint = localRay.GetPoint(hitDistance);
        if ((hitPoint - pointOnCircle).GetLengthSq() < ra2) {
          *distance = hitDistance;
          return true;
        }
      }
    }
  }
  return false;
}

bool 
IntersectTriangle( 
    const pxr::GfRay& ray, 
    const pxr::GfVec3f &a, const pxr::GfVec3f &b, const pxr::GfVec3f &c, 
    double* distance, pxr::GfVec3f* uvw) 
{ 
  pxr::GfVec3d ab(b - a); 
  pxr::GfVec3d ac(c - a);
  pxr::GfVec3d p = pxr::GfCross(ray.GetDirection(), ac);
  double det = pxr::GfDot(ab, p); 

  if (det < 0.0000001) return false; 
  double invDet = 1 / det; 

  pxr::GfVec3d t = ray.GetPoint(0.0) - pxr::GfVec3d(a); 
  (*uvw)[0] = pxr::GfDot(t, p) * invDet; 
  if ((*uvw)[0] < 0.0 || (*uvw)[0] > 1.0) return false; 

  pxr::GfVec3d q = pxr::GfCross(t, ab);
  (*uvw)[1] = pxr::GfDot(ray.GetDirection(), q) * invDet;
  if ((*uvw)[1] < 0.0 || (*uvw)[0] + (*uvw)[1] > 1.0) return false; 

  *distance = pxr::GfDot(ac, q) * invDet; 
  (*uvw)[2] = 1.0 - ((*uvw)[0] + (*uvw)[1]);

  return true; 
} 

PXR_NAMESPACE_CLOSE_SCOPE
