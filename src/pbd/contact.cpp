#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/deformable.h"
#include "../pbd/contact.h"
#include "../pbd/particle.h"
#include "../pbd/collision.h"


JVR_NAMESPACE_OPEN_SCOPE

void Contact::Init(Collision* collision, Particles* particles, size_t index)
{
  _normal = collision->GetGradient(particles, index);
  _d = collision->GetValue(particles, index);
  _hit = _d < 0;
  _velocity = collision->GetVelocity(particles, index);
  _speed = pxr::GfDot(particles->velocity[index] - _velocity, _normal);
}

void Contact::Init(SelfCollision* collision, Particles* particles, size_t index, size_t other)
{
  _compId = other;
  _geomId = index;
  _normal = collision->GetGradient(particles, index, other);
  _d = collision->GetValue(particles, index, other);
  _hit = _d < 0;
  _velocity = collision->GetVelocity(particles, index, other);
  _speed = pxr::GfDot(particles->velocity[index] - _velocity, _normal);
}

void Contact::Update(Collision* collision, Particles* particles, size_t index)
{
  _normal = collision->GetGradient(particles, index);
  _d = collision->GetValue(particles, index);
  _velocity = collision->GetVelocity(particles, index);
}

void Contact::Update(SelfCollision* collision, Particles* particles, size_t index, size_t other)
{
  _normal = collision->GetGradient(particles, index, other);
  _d = collision->GetValue(particles, index, other);
  _velocity = collision->GetVelocity(particles, index, other);
  
}

void Contacts::Resize(size_t N, size_t M) {
  if(data && n == N && m == M){ResetAllUsed(); return;}
  else if(data) {delete [] data;delete [] used;}

  n = N;
  m = M;
  data = new Contact[n * m];
  used = new int[n];
  ResetAllUsed();

};

void Contacts::ResetUsed(size_t index)
{
  used[index] = 0;
}

void 
Contacts::ResetAllUsed() { 
  memset(&used[0], 0, n * sizeof(int));
};

Contact* 
Contacts::Use(size_t index) {
  size_t available = used[index];
  used[index]++;
  return &data[index * m + available];
}

size_t 
Contacts::GetNumUsed(size_t index) const
{
  return used[index];
}

size_t 
Contacts::GetTotalNumUsed() const {
  size_t numContacts = 0;
  for(size_t x=0; x < n; ++x) numContacts += used[x];
  return numContacts;
}

JVR_NAMESPACE_CLOSE_SCOPE