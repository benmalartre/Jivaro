#include "../pbd/collision.h"
#include "../pbd/particle.h"
#include "../pbd/solver.h"

JVR_NAMESPACE_OPEN_SCOPE

/*
void Collision::AddBody(Particles* particles, Body* body)
{

}

void Collision::RemoveBody(Particles* particles, Body* body)
{

}

bool Collision::Affects(size_t index) const {
  if (!HasMask())return true;
  const size_t bitsIdx = index / sizeof(int);
  if (bitsIdx >= _mask.size())return false;
  return BITMASK_CHECK(_mask[bitsIdx], index % sizeof(int));
}
*/

PlaneCollision::PlaneCollision()
  : _position(0.f, 0.f, 0.f)
  , _normal(0.f, 1.f, 0.f)
  , _distance(0.1f)
{
}

PlaneCollision::PlaneCollision(const pxr::GfVec3f& normal, const pxr::GfVec3f& position) 
  : _position(position)
  , _normal(normal)
  , _distance(0.1f)
{
}

void PlaneCollision::FindContacts(size_t begin, size_t end, Particles* particles, const float dt)
{
  for(size_t index= begin; index < end; ++index) {
    float radius = particles->radius[index];
    float d = pxr::GfDot(_normal, particles->predicted[index]) + _distance - radius;

    if (d < 0.0) {
      //SetHit
      std::cout << "plane collision something happened !!" << std::endl;
    }
    //contacts.Add(new BodyPlaneContact3d(body, i, Normal, Distance));
  }
}

void PlaneCollision::ResolveContacts()
{
  
}


JVR_NAMESPACE_CLOSE_SCOPE