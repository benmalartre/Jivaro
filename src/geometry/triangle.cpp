#include "triangle.h"
#include "mesh.h"

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Triangle Area
//-------------------------------------------------------
float Triangle::GetArea(Mesh* mesh)
{
  const pxr::GfVec3f e0 = 
    (mesh->GetTriangleVertexPosition(this, 1) - mesh->GetTriangleVertexPosition(this, 0));
  const pxr::GfVec3f e1 = 
    (mesh->GetTriangleVertexPosition(this, 2) - mesh->GetTriangleVertexPosition(this, 1));
  const pxr::GfVec3f e2 = 
    (mesh->GetTriangleVertexPosition(this, 0) - mesh->GetTriangleVertexPosition(this, 2));

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
void Triangle::GetCenter(Mesh* mesh, pxr::GfVec3f& center)
{
  center = (
    mesh->GetTriangleVertexPosition(this, 0) +
    mesh->GetTriangleVertexPosition(this, 1) +
    mesh->GetTriangleVertexPosition(this, 2)) / 3.0f;
}

//-------------------------------------------------------
// Triangle Normal
//-------------------------------------------------------
void Triangle::GetNormal(Mesh* mesh, pxr::GfVec3f& normal)
{
  // get triangle edges
  pxr::GfVec3f AB = mesh->GetTriangleVertexPosition(this, 1) - mesh->GetTriangleVertexPosition(this, 0);
  pxr::GfVec3f AC = mesh->GetTriangleVertexPosition(this, 2) - mesh->GetTriangleVertexPosition(this, 0);
  
  // cross product
  normal = AB ^ AC;
  
  // normalize
  normal.Normalize();
}

//-------------------------------------------------------
// Triangle Closest Point
//-------------------------------------------------------
void Triangle::ClosestPoint( Mesh* mesh, const pxr::GfVec3f& point , 
  pxr::GfVec3f& closest, float& u, float& v, float& w)
{
  pxr::GfVec3f edge0 = 
    mesh->GetTriangleVertexPosition(this, 1) - mesh->GetTriangleVertexPosition(this, 0);
  pxr::GfVec3f edge1 = 
    mesh->GetTriangleVertexPosition(this, 2) - mesh->GetTriangleVertexPosition(this, 0);;
  
  pxr::GfVec3f v0 = mesh->GetTriangleVertexPosition(this, 1) - point;
  
  float a = edge0 * edge0;
  float b = edge0 * edge1;
  float c = edge1 * edge1;
  float d = edge0 * v0;
  float e = edge1 * v0;
  
  float det = a*c - b*b;
  float s = b*e - c*d;
  float t = b*d - a*e;
    
  if ( s + t < det ) {
    if ( s < 0.f ) {
      if ( t < 0.f ) {
        if ( d < 0.f ) {
          s = CLAMP( -d/a, 0.f, 1.f );
          t = 0.f;
        } else {
          s = 0.f;
          t = CLAMP( -e/c, 0.f, 1.f );
        }
      } else {
        s = 0.f;
        t = CLAMP( -e/c, 0.f, 1.f );
      }
    } else if ( t < 0.f ) {
      s = CLAMP( -d/a, 0.f, 1.f );
      t = 0.f;
    } else {
      float invDet = 1.f / det;
      s *= invDet;
      t *= invDet;
    }
  } else {
    if ( s < 0.f ) {
      float tmp0 = b+d;
      float tmp1 = c+e;
      if ( tmp1 > tmp0 ) {
        float numer = tmp1 - tmp0;
        float denom = a-2*b+c;
        s = CLAMP( numer/denom, 0.f, 1.f );
        t = 1-s;
      } else {
        t = CLAMP( -e/c, 0.f, 1.f );
        s = 0.f;
      }
    } else if ( t < 0.f ) {
      if ( a+d > b+e ) {
        float numer = c+e-b-d;
        float denom = a-2*b+c;
        s = CLAMP( numer/denom, 0.f, 1.f );
        t = 1-s;
      } else {
        s = CLAMP( -e/c, 0.f, 1.f );
        t = 0.f;
      }
    } else {
      float numer = c+e-b-d;
      float denom = a-2*b+c;
      s = CLAMP( numer/denom, 0.f, 1.f );
      t = 1.f - s;
    }
  }
  
  closest = mesh->GetTriangleVertexPosition(this, 0);
  
  v = s;
  w = t;
  u = 1.0f - v - w;
  
  closest += s * edge0 + t * edge1;
}

//-------------------------------------------------------
// Plane Box Test
//-------------------------------------------------------
bool Triangle::PlaneBoxTest(const pxr::GfVec3f& normal, 
  const pxr::GfVec3f& point, const pxr::GfVec3f& box)
{
  int q;
  
  pxr::GfVec3f vmin,vmax;
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
bool Triangle::Touch(Mesh* mesh, const pxr::GfVec3f& center, 
  const pxr::GfVec3f& boxhalfsize)
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
    
  // This is the fastest branch on Sun 
  // move everything so that the boxcenter is in (0,0,0)
  pxr::GfVec3f v0 = mesh->GetTriangleVertexPosition(this, 0) - center;
  pxr::GfVec3f v1 = mesh->GetTriangleVertexPosition(this, 1) - center;
  pxr::GfVec3f v2 = mesh->GetTriangleVertexPosition(this, 2) - center;
  
  // compute triangle edges 
  pxr::GfVec3f e0 = v1-v0;
  pxr::GfVec3f e1 = v2-v1;
  pxr::GfVec3f e2 = v0-v2;
  
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
  pxr::GfVec3f normal = e0 ^ e1;
  
  // -NJMP- (line removed here)
  if(!PlaneBoxTest(normal, v0, boxhalfsize)) return false;	// -NJMP-
  
  return true;   // box and triangle overlaps
}

PXR_NAMESPACE_CLOSE_SCOPE