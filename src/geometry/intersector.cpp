#include "intersector.h"
#include <pxr/base/gf/math.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/plane.h>

AMN_NAMESPACE_OPEN_SCOPE

template<typename SCALAR>
static inline bool IsZero(SCALAR x) {
  return pxr::GfIsClose(x, static_cast<SCALAR>(0), static_cast<SCALAR>(1e-9));
}

template<typename SCALAR>
int SolveQuadric(const SCALAR c[3], SCALAR s[2])
{
  SCALAR p, q, D;

  /* normal form: x^2 + px + q = 0 */
  p = c[1] / (2 * c[2]);
  q = c[0] / c[2];

  D = p * p - q;

  if (IsZero(D)) {
    s[0] = - p;
    return 1;
  } else if (D < 0) {
    return 0;
  } else /* if (D > 0) */ {
    SCALAR sqrt_D = sqrt(D);

    s[0] =   sqrt_D - p;
    s[1] = - sqrt_D - p;
    return 2;
  }
}

template<typename SCALAR>
int SolveCubic(const SCALAR c[4], SCALAR s[3])
{
  int i, num;
  SCALAR sub;
  SCALAR A, B, C;
  SCALAR sq_A, p, q;
  SCALAR cb_p, D;

  /* normal form: x^3 + Ax^2 + Bx + C = 0 */
  A = c[ 2 ] / c[ 3 ];
  B = c[ 1 ] / c[ 3 ];
  C = c[ 0 ] / c[ 3 ];

  /*  substitute x = y - A/3 to eliminate quadric term: x^3 +px + q = 0 */
  sq_A = A * A;
  p = 1.0/3 * (- 1.0/3 * sq_A + B);
  q = 1.0/2 * (2.0/27 * A * sq_A - 1.0/3 * A * B + C);

  /* use Cardano's formula */
  cb_p = p * p * p;
  D = q * q + cb_p;

  if (IsZero(D)) {
    if (IsZero(q)) /* one triple solution */ {
      s[ 0 ] = 0;
      num = 1;
    } else /* one single and one double solution */ {
      SCALAR u = cbrt(-q);
      s[ 0 ] = 2 * u;
      s[ 1 ] = - u;
      num = 2;
    }
  } else if (D < 0) /* Casus irreducibilis: three real solutions */ {
    SCALAR phi = 1.0/3 * acos(-q / sqrt(-cb_p));
    SCALAR t = 2 * sqrt(-p);

    s[ 0 ] =   t * cos(phi);
    s[ 1 ] = - t * cos(phi + M_PI / 3);
    s[ 2 ] = - t * cos(phi - M_PI / 3);
    num = 3;
  } else /* one real solution */ {
    SCALAR sqrt_D = sqrt(D);
    SCALAR u = cbrt(sqrt_D - q);
    SCALAR v = - cbrt(sqrt_D + q);

    s[ 0 ] = u + v;
    num = 1;
  }

  /* resubstitute */
  sub = 1.0/3 * A;

  for (i = 0; i < num; ++i)
    s[ i ] -= sub;

  return num;
}

template<typename SCALAR>
int SolveQuartic(const SCALAR c[5], SCALAR s[4])
{
  SCALAR coeffs[4];
  SCALAR z, u, v, sub;
  SCALAR A, B, C, D;
  SCALAR sq_A, p, q, r;
  int i, num = 0;

  /* normal form: x^4 + Ax^3 + Bx^2 + Cx + D = 0 */
  A = c[3] / c[4];
  B = c[2] / c[4];
  C = c[1] / c[4];
  D = c[0] / c[4];

  /*  substitute x = y - A/4 to eliminate cubic term: x^4 + px^2 + qx + r = 0 */
  sq_A = A * A;
  p = - 3.0/8 * sq_A + B;
  q = 1.0/8 * sq_A * A - 1.0/2 * A * B + C;
  r = - 3.0/256*sq_A*sq_A + 1.0/16*sq_A*B - 1.0/4*A*C + D;

  if (IsZero(r)) {
    /* no absolute term: y(y^3 + py + q) = 0 */

    coeffs[0] = q;
    coeffs[1] = p;
    coeffs[2] = 0;
    coeffs[3] = 1;

    num = SolveCubic(coeffs, s);

    s[num++] = 0;
  } else {
    /* solve the resolvent cubic ... */
    coeffs[0] = 1.0/2 * r * p - 1.0/8 * q * q;
    coeffs[1] = - r;
    coeffs[2] = - 1.0/2 * p;
    coeffs[3] = 1;

    (void) SolveCubic(coeffs, s);

    /* ... and take the one real solution ... */
    z = s[ 0 ];

    /* ... to build two quadric equations */
    u = z * z - r;
    v = 2 * z - p;

    if (IsZero(u))
        u = 0;
    else if (u > 0)
        u = sqrt(u);
    else
        return 0;

    if (IsZero(v))
        v = 0;
    else if (v > 0)
        v = sqrt(v);
    else
        return 0;

    coeffs[0] = z - u;
    coeffs[1] = q < 0 ? -v : v;
    coeffs[2] = 1;

    num = SolveQuadric(coeffs, s);

    coeffs[0]= z + u;
    coeffs[1] = q < 0 ? v : -v;
    coeffs[2] = 1;

    num += SolveQuadric(coeffs, s + num);
  }

  /* resubstitute */
  sub = 1.0/4 * A;

  for (i = 0; i < num; ++i)
	  s[i] -= sub;

  return num;
}

bool DiscIntersection(const pxr::GfRay& ray, const pxr::GfVec3d& normal, 
  const pxr::GfVec3d& center, const double radius, double* distance) 
{ 
  double hitDistance;
  if(ray.Intersect(pxr::GfPlane(normal, center), &hitDistance)) {
    pxr::GfVec3d hitPoint = ray.GetPoint(hitDistance);
    if ((hitPoint - center).GetLength() <= radius) {
      *distance = hitDistance;
      return true;
    }
  }
  return false;
} 

bool RingIntersection(const pxr::GfRay& localRay, const double radius, 
  const double section, double* distance)
{
  double hitDistance;
  if (localRay.Intersect(AMN_DEFAULT_PLANE, &hitDistance)) {
    pxr::GfVec3d hitPoint = localRay.GetPoint(hitDistance);
    double distanceFromCenter = hitPoint.GetLength();
    if (distanceFromCenter >= (radius - section) && distanceFromCenter <= (radius + section)) {
      *distance = hitDistance;
      return true;
    }
  }
  return false;
}

bool CylinderIntersection( const pxr::GfRay& localRay, const double radius, 
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

bool TorusIntersection( const pxr::GfRay& localRay, const double radius, 
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
    //float t = -n-sqrt(h); // could use this to compute intersections from ro+t*rd
  }

  // find quartic equation
  double k = (m - ra2 - Ra2) / 2.0;
  double k3 = n;
  double k2 = n * n + Ra2 * rd[1] * rd[1] + k;
  double k1 = k * n + Ra2 * ro[1] * rd[1];
  double k0 = k * k + Ra2 * ro[1] * ro[1] - Ra2 * ra2;
  
#if 1
  // prevent |c1| from being too close to zero
  if (pxr::GfAbs(k3 * (k3 * k3 - k2) + k1) < 0.01)
  {
    po = -1.0;
    double tmp = k1; k1 = k3; k3 = tmp;
    k0 = 1.0 / k0;
    k1 = k1 * k0;
    k2 = k2 * k0;
    k3 = k3 * k0;
  }
#endif
  
  double c2 = 2.0 * k2 - 3.0 * k3 * k3;
  double c1 = k3 * (k3 * k3 - k2) + k1;
  double c0 = k3 * (k3 * (-3.0 * k3 * k3 + 4.0 * k2) - 8.0 * k1) + 4.0 * k0;


  c2 /= 3.0;
  c1 *= 2.0;
  c0 /= 3.0;

  double Q = c2 * c2 + c0;
  double R = 3.0 * c0 * c2 - c2 * c2 * c2 - c1 * c1;


  double h = R * R - Q * Q * Q;
  double z = 0.0;
  if (h < 0.0)
  {
    // 4 intersections
    double sQ = pxr::GfSqrt(Q);
    z = 2.0 * sQ * pxr::GfCos(acos(R / (sQ * Q)) / 3.0);
  }
  else
  {
    // 2 intersections
    double sQ = pxr::GfPow(pxr::GfSqrt(h) + pxr::GfAbs(R), 1.0 / 3.0);
    z = pxr::GfSgn(R) * pxr::GfAbs(sQ + Q / sQ);
  }
  z = c2 - z;

  double d1 = z - 3.0 * c2;
  double d2 = z * z - 3.0 * c0;
  if (pxr::GfAbs(d1) < 1.0e-4)
  {
    if (d2 < 0.0) return false;
    d2 = pxr::GfSqrt(d2);
  }
  else
  {
    if (d1 < 0.0) return false;
    d1 = pxr::GfSqrt(d1 / 2.0);
    d2 = c1 / d1;
  }

  //----------------------------------

  double result = 1e20;

  h = d1 * d1 - z + d2;
  if (h > 0.f)
  {
    h = pxr::GfSqrt(h);
    double t1 = -d1 - h - k3; t1 = (po < 0.f) ? 2.f / t1 : t1;
    double t2 = -d1 + h - k3; t2 = (po < 0.f) ? 2.f / t2 : t2;
    if (t1 > 0.f) result = t1;
    if (t2 > 0.f) result = pxr::GfMin(result, t2);
  }

  h = d1 * d1 - z - d2;
  if (h > 0.f)
  {
    h = pxr::GfSqrt(h);
    double t1 = d1 - h - k3;  t1 = (po < 0.f) ? 2.f / t1 : t1;
    double t2 = d1 + h - k3;  t2 = (po < 0.f) ? 2.f / t2 : t2;
    if (t1 > 0.0) result = pxr::GfMin(result, t1);
    if (t2 > 0.0) result = pxr::GfMin(result, t2);
  }

  *distance = result;
  return true;
}


pxr::GfVec3d TorusNormal(const pxr::GfVec3d& point, const double radius, const double section) 
{
  double paramSquared = radius*radius + section*section;

  double x = point[0];
  double y = point[1];
  double z = point[2];
  double sumSquared = x * x + y * y + z * z;

  pxr::GfVec3d tmp(
      4.0 * x * (sumSquared - paramSquared),
      4.0 * y * (sumSquared - paramSquared + 2.0*radius*radius),
      4.0 * z * (sumSquared - paramSquared));

  return tmp.GetNormalized();
}
/*
float TorusIntersection( const pxr::GfVec3f& ro, const pxr::GfVec3f& rd, 
  float radius, float section)
{
  float po = 1.0;
  float Ra2 = radius * radius;
  float ra2 = section * section;
  float m = pxr::GfDot(ro,ro);
  float n = pxr::GfDot(ro,rd);
  float k = (m + Ra2 - ra2)/2.0;
  float k3 = n;
  pxr::GfVec2f rdxy(rd[0], rd[1]);
  pxr::GfVec2f roxy(ro[0], ro[1]);

  float k2 = n*n - Ra2*pxr::GfDot(rdxy,rdxy) + k;
  float k1 = n*k - Ra2*pxr::GfDot(rdxy,roxy);
  float k0 = k*k - Ra2*pxr::GfDot(roxy,roxy);
    
  if( pxr::GfAbs(k3*(k3*k3-k2)+k1) < 0.01 )
  {
    po = -1.0;
    float tmp=k1; k1=k3; k3=tmp;
    k0 = 1.0/k0;
    k1 = k1*k0;
    k2 = k2*k0;
    k3 = k3*k0;
  }
    
  float c2 = k2*2.0 - 3.0*k3*k3;
  float c1 = k3*(k3*k3-k2)+k1;
  float c0 = k3*(k3*(c2+2.0*k2)-8.0*k1)+4.0*k0;
  c2 /= 3.0;
  c1 *= 2.0;
  c0 /= 3.0;
  float Q = c2*c2 + c0;
  float R = c2*c2*c2 - 3.0*c2*c0 + c1*c1;
  float h = R*R - Q*Q*Q;
    
  if( h>=0.0 )  
  {
    h = sqrt(h);
    float v = pxr::GfSgn(R+h)*pow(pxr::GfAbs(R+h),1.0/3.0); // cube root
    float u = pxr::GfSgn(R-h)*pow(pxr::GfAbs(R-h),1.0/3.0); // cube root
    pxr::GfVec2f s( (v+u)+4.0*c2, (v-u)*sqrt(3.0));
    float y = sqrt(0.5*(s.GetLength()+s[0]));
    float x = 0.5*s[1]/y;
    float r = 2.0*c1/(x*x+y*y);
    float t1 =  x - r - k3; t1 = (po<0.0)?2.0/t1:t1;
    float t2 = -x - r - k3; t2 = (po<0.0)?2.0/t2:t2;
    float t = 1e20;
    if( t1>0.0 ) t=t1;
    if( t2>0.0 ) t=pxr::GfMin(t,t2);
    return t;
  }
    
    float sQ = sqrt(Q);
    float w = sQ*cos( acos(-R/(sQ*Q)) / 3.0 );
    float d2 = -(w+c2); if( d2<0.0 ) return -1.0;
    float d1 = sqrt(d2);
    float h1 = sqrt(w - 2.0*c2 + c1/d1);
    float h2 = sqrt(w - 2.0*c2 - c1/d1);
    float t1 = -d1 - h1 - k3; t1 = (po<0.0)?2.0/t1:t1;
    float t2 = -d1 + h1 - k3; t2 = (po<0.0)?2.0/t2:t2;
    float t3 =  d1 - h2 - k3; t3 = (po<0.0)?2.0/t3:t3;
    float t4 =  d1 + h2 - k3; t4 = (po<0.0)?2.0/t4:t4;
    float t = 1e20;
    if( t1>0.0 ) t=t1;
    if( t2>0.0 ) t=pxr::GfMin(t,t2);
    if( t3>0.0 ) t=pxr::GfMin(t,t3);
    if( t4>0.0 ) t=pxr::GfMin(t,t4);
    return t;
}

pxr::GfVec3f TorusNormal( const pxr::GfVec3f& pos, 
  float radius, float section )
{
  return pxr::GfVec3f(0.f, 1.f, 0.f);
  
  float s2 = section * section;
  float r2 = radius * radius;
  return pos * (pxr::GfDot(pos, pos) - 
    section * section - 
    radius * radius * pxr::GfVec3f(1.f, 1.f, -1.f)).GetNormalized();
}
*/
AMN_NAMESPACE_CLOSE_SCOPE