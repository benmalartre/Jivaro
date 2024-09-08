
#include "../acceleration/bvh.h"
#include "../acceleration/gradient.h"
#include "../geometry/smooth.h"
#include "../geometry/mesh.h"


JVR_NAMESPACE_OPEN_SCOPE




void Gradient::Init(Mesh* mesh)
{
  size_t numPoints = mesh->GetNumPoints();
  _flags.resize(numPoints);
  _gradient.resize(numPoints);
  _value.resize(numPoints);
}
  
void Gradient::Compute(Mesh* mesh)
{

}

void Gradient::SetSeed(pxr::VtArray<int> &seed)
{
  for(size_t index = 0; index < seed.size(); ++index)
    BIT_SET(_flags[index], Gradient::SEED);
}

void Gradient::SetFixed(pxr::VtArray<int> &fixed)
{
  for(size_t index = 0; index < fixed.size(); ++index)
    BIT_SET(_flags[index], Gradient::FIXED);
}

void Gradient::_FindFeatures(Mesh* mesh)
{
  BVH bvh;
  bvh.Init({ mesh });


}



JVR_NAMESPACE_CLOSE_SCOPE
