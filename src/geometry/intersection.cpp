#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/plane.h>
#include "../geometry/intersection.h"

JVR_NAMESPACE_OPEN_SCOPE
//=================================================================================================
// INTERSECTION ROUTINES
//=================================================================================================
bool 
IntersectDisc(const GfRay& localRay, const double radius, double* distance) 
{ 
  double hitDistance;
  if(localRay.Intersect(DEFAULT_PLANE, &hitDistance)) {
    GfVec3d hitPoint = localRay.GetPoint(hitDistance);
    if (hitPoint.GetLength() <= radius) {
      *distance = hitDistance;
      return true;
    }
  }
  return false;
} 

bool 
IntersectRing(const GfRay& localRay, const double radius, 
  const double section, double* distance)
{
  double hitDistance;
  if (localRay.Intersect(DEFAULT_PLANE, &hitDistance)) {
    GfVec3d hitPoint = localRay.GetPoint(hitDistance);
    double distanceFromCenter = hitPoint.GetLength();
    if (distanceFromCenter >= (radius - section) && distanceFromCenter <= (radius + section)) {
      *distance = hitDistance;
      return true;
    }
  }
  return false;
}

bool 
IntersectCylinder( const GfRay& localRay, const double radius, 
  const double height, double* distance)
{
  double enterDistance, exitDistance;
  const GfVec3d axis(0, 1, 0);
  if(localRay.Intersect(GfVec3d(0.f), axis,
    radius, &enterDistance, &exitDistance)) {
      // check enter point
      GfVec3d hit = localRay.GetPoint(enterDistance);
      GfVec3d projected = hit.GetProjection(axis);
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
IntersectTube(const GfRay& localRay, const double innerRadius,
  const double outerRadius, const double height, double* distance)
{
  // closest point on circle
  double hitDistance;
  double radius = (innerRadius + outerRadius) * 0.5;
  double section = (outerRadius - innerRadius) * 0.5;
  if (localRay.Intersect(DEFAULT_PLANE, &hitDistance)) {
    GfVec3d hitPoint = localRay.GetPoint(hitDistance);
    double hitLength = hitPoint.GetLength();
    if (hitLength > innerRadius && hitLength < outerRadius) {
      *distance = hitDistance - section;
      return true;
    }
    hitPoint.Normalize();
    // check tangent plane
    GfVec3d pointOnCircle(hitPoint * radius);
    GfPlane tangentPlane(
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
    GfPlane biTangentPlane(
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
IntersectTorus( const GfRay& localRay, const double radius, 
  const double section, double* distance)
{
  
  double po = 1.0;

  const GfVec3d ro(localRay.GetPoint(0));
  const GfVec3d rd((localRay.GetPoint(1) - ro).GetNormalized());
  double Ra2 = radius * radius;
  double ra2 = section * section;

  double m = GfDot(ro, ro);
  double n = GfDot(ro, rd);

  // bounding sphere
  {
    double h = n * n - m + (radius + section) * (radius + section);
    if (h < 0.0) return false;
  }

  double k = (m + Ra2 - ra2) / 2.0;
  double k3 = n;
  const GfVec2d rdxz(rd[0], rd[2]);
  const GfVec2d roxz(ro[0], ro[2]);
  double k2 = n * n - Ra2 * GfDot(rdxz, rdxz) + k;
  double k1 = n * k - Ra2 * GfDot(rdxz, roxz);
  double k0 = k * k - Ra2 * GfDot(roxz, roxz);

  if (GfAbs(k3 * (k3 * k3 - k2) + k1) < 0.01)
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
    h = GfSqrt(h);
    double v = GfSgn(R + h) * GfPow(GfAbs(R + h), 1.0 / 3.0); // cube root
    double u = GfSgn(R - h) * GfPow(GfAbs(R - h), 1.0 / 3.0); // cube root
    GfVec2d s((v + u) + 4.0 * c2, (v - u) * sqrt(3.0));
    double z = GfSqrt(0.5 * (s.GetLength() + s[0]));
    double x = 0.5 * s[1] / z;
    double r = 2.0 * c1 / (x * x + z * z);
    double t1 = x - r - k3; t1 = (po < 0.0) ? 2.0 / t1 : t1;
    double t2 = -x - r - k3; t2 = (po < 0.0) ? 2.0 / t2 : t2;
    double t = 1e20;
    if (t1 > 0.0) t = t1;
    if (t2 > 0.0) t = GfMin(t, t2);
    *distance = t;
    return true;
  }

  double sQ = GfSqrt(Q);
  double w = sQ * GfCos(acos(-R / (sQ * Q)) / 3.0);
  double d2 = -(w + c2); if (d2 < 0.0) return false;
  double d1 = GfSqrt(d2);
  double h1 = GfSqrt(w - 2.0 * c2 + c1 / d1);
  double h2 = GfSqrt(w - 2.0 * c2 - c1 / d1);
  double t1 = -d1 - h1 - k3; t1 = (po < 0.0) ? 2.0 / t1 : t1;
  double t2 = -d1 + h1 - k3; t2 = (po < 0.0) ? 2.0 / t2 : t2;
  double t3 = d1 - h2 - k3; t3 = (po < 0.0) ? 2.0 / t3 : t3;
  double t4 = d1 + h2 - k3; t4 = (po < 0.0) ? 2.0 / t4 : t4;
  double t = 1e20;
  if (t1 > 0.0) t = t1;
  if (t2 > 0.0) t = GfMin(t, t2);
  if (t3 > 0.0) t = GfMin(t, t3);
  if (t4 > 0.0) t = GfMin(t, t4);
  *distance = t;
  return true;
}

bool 
IntersectTorusApprox(const GfRay& localRay, const double radius,
  const double section, double* distance)
{
  const GfVec3d ro(localRay.GetPoint(0));
  const GfVec3d rd((localRay.GetPoint(1) - ro).GetNormalized());
  double Ra2 = radius * radius;
  double ra2 = section * section;

  double m = GfDot(ro, ro);
  double n = GfDot(ro, rd);

  // bounding sphere
  {
    double h = n * n - m + (radius + section) * (radius + section);
    if (h < 0.0) return false;
  }

  // closest point on circle
  double hitDistance;
  if (localRay.Intersect(DEFAULT_PLANE, &hitDistance)) {
    GfVec3d hitPoint = localRay.GetPoint(hitDistance);
    GfVec3d pointOnCircle(hitPoint.GetNormalized() * radius);
    if ((hitPoint - pointOnCircle).GetLengthSq() < ra2) {
      *distance = hitDistance - section;
      return true;
    }
    else {
      GfPlane hitCrossPlane(
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
    const GfRay& ray, 
    const GfVec3f &a, const GfVec3f &b, const GfVec3f &c, 
    double* distance, GfVec3f* uvw) 
{ 
  GfVec3d ab(b - a); 
  GfVec3d ac(c - a);
  GfVec3d p = GfCross(ray.GetDirection(), ac);
  double det = GfDot(ab, p); 

  if (det < 0.0000001) return false; 
  double invDet = 1 / det; 

  GfVec3d t = ray.GetPoint(0.0) - GfVec3d(a); 
  (*uvw)[0] = GfDot(t, p) * invDet; 
  if ((*uvw)[0] < 0.0 || (*uvw)[0] > 1.0) return false; 

  GfVec3d q = GfCross(t, ab);
  (*uvw)[1] = GfDot(ray.GetDirection(), q) * invDet;
  if ((*uvw)[1] < 0.0 || (*uvw)[0] + (*uvw)[1] > 1.0) return false; 

  *distance = GfDot(ac, q) * invDet; 
  (*uvw)[2] = 1.0 - ((*uvw)[0] + (*uvw)[1]);

  return true; 
} 

PXR_NAMESPACE_CLOSE_SCOPE
