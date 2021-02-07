#include "intersector.h"
#include <pxr/base/gf/math.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/plane.h>

AMN_NAMESPACE_OPEN_SCOPE

template<typename T>
static inline bool IsZero(T x) {
  return pxr::GfIsClose(x, 0, 1e-9);
}

template<typename T>
int SolveQuadric(const T c[3], T s[2])
{
  T p, q, D;

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
    T sqrt_D = sqrt(D);

    s[0] =   sqrt_D - p;
    s[1] = - sqrt_D - p;
    return 2;
  }
}

template<typename T>
int SolveCubic(const T c[4], T s[3])
{
  int i, num;
  T   sub;
  T   A, B, C;
  T   sq_A, p, q;
  T   cb_p, D;

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
      T u = cbrt(-q);
      s[ 0 ] = 2 * u;
      s[ 1 ] = - u;
      num = 2;
    }
  } else if (D < 0) /* Casus irreducibilis: three real solutions */ {
    T phi = 1.0/3 * acos(-q / sqrt(-cb_p));
    T t = 2 * sqrt(-p);

    s[ 0 ] =   t * cos(phi);
    s[ 1 ] = - t * cos(phi + M_PI / 3);
    s[ 2 ] = - t * cos(phi - M_PI / 3);
    num = 3;
  } else /* one real solution */ {
    T sqrt_D = sqrt(D);
    T u = cbrt(sqrt_D - q);
    T v = - cbrt(sqrt_D + q);

    s[ 0 ] = u + v;
    num = 1;
  }

  /* resubstitute */
  sub = 1.0/3 * A;

  for (i = 0; i < num; ++i)
    s[ i ] -= sub;

  return num;
}

template<typename T>
int SolveQuartic(const T c[5], T s[4])
{
  T   coeffs[4];
  T   z, u, v, sub;
  T   A, B, C, D;
  T   sq_A, p, q, r;
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

float DiscIntersection(const pxr::GfRay& ray, const pxr::GfVec3d& normal, 
  const pxr::GfVec3d& center, float radius) 
{ 
  double hitDistance;
  if(ray.Intersect(pxr::GfPlane(normal, center), &hitDistance)) {
    pxr::GfVec3d hitPoint = ray.GetPoint(hitDistance);
    if((hitPoint - center).GetLength() <= radius) return hitDistance;
  }
  return -1.f;
} 

float CylinderIntersection( const pxr::GfRay& ray, float radius, float height)
{
  double enterDistance, exitDistance;
  const pxr::GfVec3d axis(0, 1, 0);
  if(ray.Intersect(pxr::GfVec3d(0.f), axis,
    radius, &enterDistance, &exitDistance)) {
      // check enter point
      pxr::GfVec3d hit = ray.GetPoint(enterDistance);
      pxr::GfVec3d projected = hit.GetProjection(axis);
      if(hit[1] >= 0.f && hit[1] <= height) return enterDistance;
      // check exit point
      hit = ray.GetPoint(exitDistance);
      projected = hit.GetProjection(axis);
      if(hit[1] >= 0.f && hit[1] <= height) return exitDistance;
  }
  return -1.f;
}

float TorusIntersection( const pxr::GfRay& ray, float radius, float section)
{
  const pxr::GfVec3d& origin = ray.GetPoint(0);
  double ox = origin[0];
  double oy = origin[1];
  double oz = origin[2];

  const pxr::GfVec3d& direction = ray.GetDirection();
  double dx = direction[0];
  double dy = direction[1];
  double dz = direction[2];

  // define the coefficients of the quartic equation
  double sum_d_sqrd = dx * dx + dy * dy + dz * dz;
  double e = ox * ox + oy * oy + oz * oz - radius*radius - section*section;
  double f = ox * dx + oy * dy + oz * dz;
  double four_a_sqrd = 4.0 * radius * radius;

  double coeffs[5] = {
    e * e - four_a_sqrd * (section*section - oy * oy),
    4.f * f * e + 2.f * four_a_sqrd * oy * dy,
    2.f * sum_d_sqrd * e + 4.f * f * f + four_a_sqrd * dy * dy,
    4.f * sum_d_sqrd * f,
    sum_d_sqrd * sum_d_sqrd
  };
  double roots[4] = {0.f, 0.f, 0.f, 0.f};

  int numRealRoots = SolveQuartic(coeffs, roots);
  if(numRealRoots) {
    // find the smallest root greater than kEpsilon, if any
    // the roots array is not sorted
    bool intersected = false;
    float t = std::numeric_limits<float>::max();

    for (int j = 0; j < numRealRoots; ++j)  
      if (roots[j] > 0.000001f) {
        intersected = true;
        if (roots[j] < t)
          t = roots[j];
      }
    if(intersected) return t;
  }
  return -1.f;
}


pxr::GfVec3d TorusNormal(const pxr::GfVec3d& point, float radius, float section) 
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