#include "triangle.h"
#include "mesh.h"

AMN_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------
// Triangle Area
//-------------------------------------------------------
double Triangle::getArea(Mesh* mesh)
{
	const MVector AB = (mesh->getPosition(this, 1) - mesh->getPosition(this, 0));
	const MVector BC = (mesh->getPosition(this, 2) - mesh->getPosition(this, 1));
	const MVector CA = (mesh->getPosition(this, 0) - mesh->getPosition(this, 2));

	// edge length
	double lAB = AB.length();
	double lBC = BC.length();
	double lCA = CA.length();

	// half perimeter
	double P = (lAB + lBC + lCA) * 0.5;

	// compute area
	return sqrt(P*(P - lAB)*(P - lBC)*(P - lCA));
}

//-------------------------------------------------------
// Triangle Center
//-------------------------------------------------------
void Triangle::getCenter(Mesh* mesh, MVector& center)
{
    center = (mesh->getPosition(this, 0)+ mesh->getPosition(this, 1)+ mesh->getPosition(this, 2))/3.0f;
}

//-------------------------------------------------------
// Triangle Normal
//-------------------------------------------------------
void Triangle::getNormal(Mesh* mesh, MVector& normal)
{
    // get triangle edges
    MVector AB = mesh->getPosition(this, 1) - mesh->getPosition(this, 0);
    MVector AC = mesh->getPosition(this, 2) - mesh->getPosition(this, 0);
    
    // cross product
    normal = AB^AC;
    
    // normalize
    normal.normalize();
}

//-------------------------------------------------------
// Triangle Closest Point
//-------------------------------------------------------
void Triangle::closestPoint( Mesh* mesh, const MVector &point , MVector& closest, float& u, float& v, float& w)
{
    MVector edge0 = mesh->getPosition(this, 1) - mesh->getPosition(this, 0);
    MVector edge1 = mesh->getPosition(this, 2) - mesh->getPosition(this, 0);;
    
    MVector v0 = mesh->getPosition(this, 1) - point;
    
    float a = edge0 * edge0;
    float b = edge0 * edge1;
    float c = edge1 * edge1;
    float d = edge0 * v0;
    float e = edge1 * v0;
    
    float det = a*c - b*b;
    float s = b*e - c*d;
    float t = b*d - a*e;
    
    if ( s + t < det )
    {
        if ( s < 0.f )
        {
            if ( t < 0.f )
            {
                if ( d < 0.f )
                {
                    s = CLAMP( -d/a, 0.f, 1.f );
                    t = 0.f;
                }
                else
                {
                    s = 0.f;
                    t = CLAMP( -e/c, 0.f, 1.f );
                }
            }
            else
            {
                s = 0.f;
                t = CLAMP( -e/c, 0.f, 1.f );
            }
        }
        else if ( t < 0.f )
        {
            s = CLAMP( -d/a, 0.f, 1.f );
            t = 0.f;
        }
        else
        {
            float invDet = 1.f / det;
            s *= invDet;
            t *= invDet;
        }
    }
    else
    {
        if ( s < 0.f )
        {
            float tmp0 = b+d;
            float tmp1 = c+e;
            if ( tmp1 > tmp0 )
            {
                float numer = tmp1 - tmp0;
                float denom = a-2*b+c;
                s = CLAMP( numer/denom, 0.f, 1.f );
                t = 1-s;
            }
            else
            {
                t = CLAMP( -e/c, 0.f, 1.f );
                s = 0.f;
            }
        }
        else if ( t < 0.f )
        {
            if ( a+d > b+e )
            {
                float numer = c+e-b-d;
                float denom = a-2*b+c;
                s = CLAMP( numer/denom, 0.f, 1.f );
                t = 1-s;
            }
            else
            {
                s = CLAMP( -e/c, 0.f, 1.f );
                t = 0.f;
            }
        }
        else
        {
            float numer = c+e-b-d;
            float denom = a-2*b+c;
            s = CLAMP( numer/denom, 0.f, 1.f );
            t = 1.f - s;
        }
    }
    
    closest = mesh->getPosition(this, 0);
    
    v = s;
    w = t;
    u = 1.0f - v - w;
    
    closest += s * edge0 + t * edge1;
}

//-------------------------------------------------------
// Plane Box Test
//-------------------------------------------------------
bool Triangle::planeBoxTest(const MVector& normal, const MVector& vert, const MVector& maxbox)
{
    int q;
    
    MVector vmin,vmax;
    float v;
    
    for(q=0;q<=2;q++)
    {
        
        v=vert[q];
        if(normal[q]>0.0f)
            
        {
            vmin[q]=-maxbox[q] - v;
            vmax[q]= maxbox[q] - v;
        }
        
        else
            
        {
            vmin[q]= maxbox[q] - v;
            vmax[q]=-maxbox[q] - v;
        }
        
    }
    
    if((normal*vmin)>0.0f) return false;
    if((normal*vmax)>=0.0f) return true;
    
    return false;
}

//-------------------------------------------------------
// Triangle Intersect Bounding Box
//-------------------------------------------------------
bool Triangle::touch(Mesh* mesh, const MVector& center, const MVector& boxhalfsize)
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
    
    /* This is the fastest branch on Sun */
    /* move everything so that the boxcenter is in (0,0,0) */
    MVector v0 = mesh->getPosition(this, 0) - center;
    MVector v1 = mesh->getPosition(this, 1) - center;
    MVector v2 = mesh->getPosition(this, 2) - center;
    
    /* compute triangle edges */
    MVector e0 = v1-v0;
    MVector e1 = v2-v1;
    MVector e2 = v0-v2;
    
    /*  test the 9 tests first (this was faster) */
    fex = fabs(e0.x);
    fey = fabs(e0.y);
    fez = fabs(e0.z);
    
    AXISTEST_X01(e0.z, e0.y, fez, fey);
    AXISTEST_Y02(e0.z, e0.x, fez, fex);
    AXISTEST_Z12(e0.y, e0.x, fey, fex);
    
    fex = fabs(e1.x);
    fey = fabs(e1.y);
    fez = fabs(e1.z);
    
    AXISTEST_X01(e1.z, e1.y, fez, fey);
    AXISTEST_Y02(e1.z, e1.x, fez, fex);
    AXISTEST_Z0(e1.y, e1.x, fey, fex);
    
    fex = fabs(e2.x);
    fey = fabs(e2.y);
    fez = fabs(e2.z);
    
    AXISTEST_X2(e2.z, e2.y, fez, fey);
    AXISTEST_Y1(e2.z, e2.x, fez, fex);
    AXISTEST_Z12(e2.y, e2.x, fey, fex);
    
    /*
     first test overlap in the {x,y,z}-directions
     find min, max of the triangle each direction, and test for overlap in
     that direction -- this is equivalent to testing a minimal AABB around
     the triangle against the AABB
     */
    
    // test in X-direction
    FINDMINMAX(v0.x,v1.x,v2.x,min,max);
    if(min>boxhalfsize.x || max<-boxhalfsize.x) return false;
    
    // test in Y-direction
    FINDMINMAX(v0.y,v1.y,v2.y,min,max);
    if(min>boxhalfsize.y || max<-boxhalfsize.y) return false;
    
    // test in Z-direction
    FINDMINMAX(v0.z,v1.z,v2.z,min,max);
    if(min>boxhalfsize.z || max<-boxhalfsize.z) return false;
    
    /*
     test if the box intersects the plane of the triangle
     compute plane equation of triangle: normal*x+d=0
     */
    MVector normal = e0^e1;
    
    // -NJMP- (line removed here)
    if(!planeBoxTest(normal,v0,boxhalfsize)) return false;	// -NJMP-
    
    return true;   /* box and triangle overlaps */
}

AMN_NAMESPACE_CLOSE_SCOPE