#include <algorithm>
#include "../geometry/utils.h"
#include "../geometry/triangle.h"
#include "../geometry/mesh.h"
#include "../acceleration/intersector.h"

JVR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Triangle Area
//-------------------------------------------------------
float Triangle::GetArea(const GfVec3f* points) const
{

  const GfVec3f e0 = points[vertices[1]] - points[vertices[0]];
  const GfVec3f e1 = points[vertices[2]] - points[vertices[1]];
  const GfVec3f e2 = points[vertices[0]] - points[vertices[2]];

  // edge length
  float le0 = e0.GetLength();
  float le1 = e1.GetLength();
  float le2 = e2.GetLength();

  // half perimeter
  float hp = (le0 + le1 + le2) * 0.5;

  // compute area
  return sqrt(hp * (hp - le0) * ( hp - le1) * (hp - le2));
}

//-------------------------------------------------------
// Triangle Center
//-------------------------------------------------------
GfVec3f 
Triangle::GetCenter(const GfVec3f* points) const
{
  return GfVec3f(
    points[vertices[0]] +
    points[vertices[1]] +
    points[vertices[2]]) / 3.0f;
}

//-------------------------------------------------------
// Triangle Normal
//-------------------------------------------------------
GfVec3f 
Triangle::GetNormal(const GfVec3f* points) const
{

  // get triangle edges
  GfVec3f AB = points[vertices[1]]- points[vertices[0]];
  GfVec3f AC = points[vertices[2]] - points[vertices[0]];
  
  // cross product
  GfVec3f normal = AB ^ AC;
  
  // normalize
  normal.Normalize();
  return normal;
}

//-------------------------------------------------------
// Triangle Velocity
//-------------------------------------------------------
GfVec3f 
Triangle::GetVelocity(const GfVec3f* positions, const GfVec3f* previous) const
{
  const GfVec3f prevPos = GetCenter(previous);
  const GfVec3f curPos = GetCenter(positions);
  const GfVec3f prevNormal = GetNormal(previous);
  const GfVec3f curNormal = GetNormal(positions);

  const GfVec3f deltaP(curPos - prevPos);
  GfVec3f velocity = GfVec3f(deltaP / Geometry::FrameDuration);

  const GfQuatf deltaR = GetRotationBetweenVectors(prevNormal, curNormal);
  
  const GfVec3f torque = GfVec3f(deltaR.GetImaginary() * (deltaR.GetReal() / Geometry::FrameDuration));

  const GfVec3f tangent = (curNormal ^ torque).GetNormalized();

  return velocity + tangent * torque.GetLength();
}

//-------------------------------------------------------
// Triangle bounding box
//-------------------------------------------------------
GfRange3f 
Triangle::GetBoundingBox(const GfVec3f* positions, const GfMatrix4d& m) const
{
  GfRange3f range;

  range.UnionWith(GfVec3f(m.Transform(positions[vertices[0]])));
  range.UnionWith(GfVec3f(m.Transform(positions[vertices[1]])));
  range.UnionWith(GfVec3f(m.Transform(positions[vertices[2]])));
 
  return range;
}

bool Triangle::Raycast(const GfVec3f* points, const GfRay& ray, Location* hit) const
{
  GfVec3d baryCoords;
  double distance;
  bool frontFacing;

  if (ray.Intersect(
    points[vertices[0]],
    points[vertices[1]],
    points[vertices[2]],
    &distance, &baryCoords, &frontFacing/*, maxDistance*/)) {

    hit->SetComponentIndex(id);
    hit->SetCoordinates(baryCoords);
    hit->SetDistance(distance);
    return true;
  }
  return false;
}


//-------------------------------------------------------
// Triangle Closest Point
//-------------------------------------------------------
static inline float _Dot2( const GfVec3f& v ) 
{ 
  return GfDot(v,v); 
}

bool Triangle::Closest(const GfVec3f* points, const GfVec3f& point, Location* hit) const
{
  GfVec3f edge0 = points[vertices[1]] - points[vertices[0]];
  GfVec3f edge1 = points[vertices[2]] - points[vertices[0]];
  GfVec3f v0 = points[vertices[0]] - point;
  GfVec3f closest = points[vertices[0]];

  double a = edge0 * edge0;
  double b = edge0 * edge1;
  double c = edge1 * edge1;
  double d = edge0 * v0;
  double e = edge1 * v0;
  
  double det = a*c - b*b;
  double s = b*e - c*d;
  double t = b*d - a*e;
    
  if ( s + t < det ) {
    if ( s < 0.0 ) {
      if ( t < 0.0 ) {
        if ( d < 0.0 ) {
          s = CLAMP( -d/a, 0.0, 1.0 );
          t = 0.0;
        } else {
          s = 0.0;
          t = CLAMP( -e/c, 0.0, 1.0 );
        }
      } else {
        s = 0.0;
        t = CLAMP( -e/c, 0.0, 1.0 );
      }
    } else if ( t < 0.0 ) {
      s = CLAMP( -d/a, 0.0, 1.0 );
      t = 0.0;
    } else {
      double invDet = 1.0 / det;
      s *= invDet;
      t *= invDet;
    }
  } else {
    if ( s < 0.0 ) {
      double tmp0 = b+d;
      double tmp1 = c+e;
      if ( tmp1 > tmp0 ) {
        double numer = tmp1 - tmp0;
        double denom = a-2*b+c;
        s = CLAMP( numer/denom, 0.0, 1.0 );
        t = 1-s;
      } else {
        t = CLAMP( -e/c, 0.0, 1.0 );
        s = 0.0;
      }
    } else if ( t < 0.0 ) {
      if ( a+d > b+e ) {
        double numer = c+e-b-d;
        double denom = a-2*b+c;
        s = CLAMP( numer/denom, 0.0, 1.0 );
        t = 1-s;
      } else {
        s = CLAMP( -e/c, 0.0, 1.0 );
        t = 0.0;
      }
    } else {
      double numer = c+e-b-d;
      double denom = a-2*b+c;
      s = CLAMP( numer/denom, 0.0, 1.0 );
      t = 1.0 - s;
    }
  }
  
  closest += s * edge0 + t * edge1;
  double lastDistanceSq = (point - hit->GetPoint()).GetLengthSq();
  double distanceSq = (point - closest).GetLengthSq();
  if(distanceSq < lastDistanceSq) {
    hit->SetCoordinates(GfVec3f(1.0 - s - t, s, t));
    hit->SetComponentIndex(id);
    hit->SetPoint(closest);          // local point 
    return true;
  }
  return false;
}

//-------------------------------------------------------
// Plane Box Test
//-------------------------------------------------------
bool Triangle::PlaneBoxTest(const GfVec3f& normal, 
  const GfVec3f& point, const GfVec3f& box) const
{
  int q;
  
  GfVec3f vmin,vmax;
  float v;
  
  for(q=0; q <= 2; ++q) {
    v=point[q];
    if(normal[q]>0.0f) {
      vmin[q]=-box[q] - v;
      vmax[q]= box[q] - v;
    } else {
      vmin[q]= box[q] - v;
      vmax[q]=-box[q] - v;
    }
  }
  
  if((normal * vmin) >  0.0f) return false;
  if((normal * vmax) >= 0.0f) return true;
  
  return false;
}

//-------------------------------------------------------
// Triangle Intersect Bounding Box
//-------------------------------------------------------
bool Triangle::Touch(const GfVec3f* points, const GfVec3f& center, 
  const GfVec3f& boxhalfsize) const
{
  /*
  use separating axis theorem to test overlap between triangle and box
  need to test for overlap in these directions:
  
  1) the {x,y,z}-directions (actually, since we use the AABB of the triangle
  we do not even need to test these)
  2) normal of the triangle
  3) crossproduct(edge from triangle, {x,y,z}-direction)
  
  this gives 3x3=9 more tests
  */

  float min,max,p0,p1,p2,rad,fex,fey,fez;
    
  // move everything so that the boxcenter is in (0,0,0)
  GfVec3f v0 = points[vertices[0]] - center;
  GfVec3f v1 = points[vertices[1]] - center;
  GfVec3f v2 = points[vertices[2]] - center;
  
  // compute triangle edges 
  GfVec3f e0 = v1-v0;
  GfVec3f e1 = v2-v1;
  GfVec3f e2 = v0-v2;
  
  // test the 9 tests first (this was faster)
  fex = fabs(e0[0]);
  fey = fabs(e0[1]);
  fez = fabs(e0[2]);
  
  AXISTEST_X01(e0[2], e0[1], fez, fey);
  AXISTEST_Y02(e0[2], e0[0], fez, fex);
  AXISTEST_Z12(e0[1], e0[0], fey, fex);
    
  fex = fabs(e1[0]);
  fey = fabs(e1[1]);
  fez = fabs(e1[2]);
  
  AXISTEST_X01(e1[2], e1[1], fez, fey);
  AXISTEST_Y02(e1[2], e1[0], fez, fex);
  AXISTEST_Z0(e1[1], e1[0], fey, fex);
  
  fex = fabs(e2[0]);
  fey = fabs(e2[1]);
  fez = fabs(e2[2]);
  
  AXISTEST_X2(e2[2], e2[1], fez, fey);
  AXISTEST_Y1(e2[2], e2[0], fez, fex);
  AXISTEST_Z12(e2[1], e2[0], fey, fex);
  
  // first test overlap in the {x,y,z}-directions
  // find min, max of the triangle each direction, and test for overlap in
  // that direction -- this is equivalent to testing a minimal AABB around
  // the triangle against the AABB
  
  // test in XYZ directions
  for(size_t axis=0; axis < 3; ++axis) {
    FINDMINMAX(v0[axis], v1[axis], v2[axis], min, max);
    if(min>boxhalfsize[axis] || max<-boxhalfsize[axis]) return false;
  }
  
  // test if the box intersects the plane of the triangle
  // compute plane equation of triangle: normal*x+d=0
  GfVec3f normal = e0 ^ e1;
  
  if(!PlaneBoxTest(normal, v0, boxhalfsize)) return false;
  
  return true;   // box and triangle overlaps
}

//-------------------------------------------------------
// TrianglePair vertices
//-------------------------------------------------------
static bool _IsVertexShared(size_t vertex, const GfVec3i& tri1, const GfVec3i& tri2)
{
  bool check = false;
  for(size_t i = 0; i < 3; ++i) {
    if(tri1[i] == vertex){check=true; break;};
  }
  if(!check)return false;
  for(size_t i = 0; i < 3; ++i) {
    if(tri2[i] == vertex){return true;};
  }
  return false;
}

//-------------------------------------------------------
// TrianglePair vertices
//-------------------------------------------------------
GfVec4i 
TrianglePair::GetVertices() const
{
  if (!right)
    return GfVec4i(left->vertices[0], left->vertices[1], left->vertices[2], -1);

  size_t sharedIdx = 0;
  GfVec4i vertices(-1);

  // left triangle
  for(size_t vertexIdx = 0; vertexIdx < 3; ++vertexIdx) {
    if(_IsVertexShared(left->vertices[vertexIdx], left->vertices, right->vertices)) {
      vertices[sharedIdx++] = left->vertices[vertexIdx];
    } else {
      vertices[2] = left->vertices[vertexIdx];
    }
  }

   // right triangle
  for(size_t vertexIdx = 0; vertexIdx < 3; ++vertexIdx) {
    if(_IsVertexShared(right->vertices[vertexIdx], left->vertices, right->vertices))
      continue;
    vertices[3] = right->vertices[vertexIdx];
    break;
  }

  if(vertices[0] > vertices[1])
    vertices = GfVec4i(vertices[1], vertices[0], vertices[2], vertices[3]);

  return vertices;
}

//-------------------------------------------------------
// TrianglePair bounding box
//-------------------------------------------------------
GfRange3f 
TrianglePair::GetBoundingBox(const GfVec3f* positions, const GfMatrix4d& m) const
{
  GfRange3f range;

  range.UnionWith(GfVec3f(m.Transform(positions[left->vertices[0]])));
  range.UnionWith(GfVec3f(m.Transform(positions[left->vertices[1]])));
  range.UnionWith(GfVec3f(m.Transform(positions[left->vertices[2]])));

  if (right) {
    range.UnionWith(GfVec3f(m.Transform(positions[right->vertices[0]])));
    range.UnionWith(GfVec3f(m.Transform(positions[right->vertices[1]])));
    range.UnionWith(GfVec3f(m.Transform(positions[right->vertices[2]])));
  }

  return range;
}

//-------------------------------------------------------
// TrianglePair raycast
//-------------------------------------------------------
bool
TrianglePair::Raycast(const GfVec3f* points, const GfRay& ray, Location* hit) const
{
  bool hitSometing = false;
  if (left->Raycast(points, ray, hit))hitSometing = true;
  if (right && right->Raycast(points, ray, hit))hitSometing = true;
  return hitSometing;
}

//-------------------------------------------------------
// TrianglePair closest point
//-------------------------------------------------------
bool 
TrianglePair::Closest(const GfVec3f* points, const GfVec3f& point, Location* hit) const
{
  bool hitSometing = false;
  if (left->Closest(points, point, hit))hitSometing = true;
  if (right && right->Closest(points, point, hit))hitSometing = true;
  return hitSometing;
};

//-------------------------------------------------------
// TrianglePair touch box
//-------------------------------------------------------
bool 
TrianglePair::Touch(const GfVec3f* points, const GfVec3f& center, 
  const GfVec3f& boxhalfsize) const
{
  return false;
}

static size_t
GetLongestEdgeInTriangle(const GfVec3i& vertices, const GfVec3f* positions)
{
  const float edge0 = (positions[vertices[1]] - positions[vertices[0]]).GetLengthSq();
  const float edge1 = (positions[vertices[2]] - positions[vertices[1]]).GetLengthSq();
  const float edge2 = (positions[vertices[0]] - positions[vertices[2]]).GetLengthSq();
  if (edge0 > edge1 && edge0 > edge2)return 0;
  else if (edge1 > edge0 && edge1 > edge2)return 1;
  else return 2;
}


JVR_NAMESPACE_CLOSE_SCOPE