#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/deformable.h"
#include "../pbd/contact.h"
#include "../pbd/particle.h"
#include "../pbd/collision.h"


JVR_NAMESPACE_OPEN_SCOPE

void Contact::Init(Collision* collision, Particles* particles, size_t index, size_t other)
{
  _compId = other;
  _geomId = other;
  _normal = collision->GetGradient(particles, index);
  _d = collision->GetValue(particles, index);
  _velocity = collision->GetVelocity(particles, index);
  _speed = pxr::GfDot(particles->velocity[index] - _velocity, _normal);
}

void Contact::Update(Collision* collision, Particles* particles, size_t index)
{
  _normal = collision->GetGradient(particles, index);
  _d = collision->GetValue(particles, index);
  _velocity = collision->GetVelocity(particles, index);
  
}


void Contacts::Resize(size_t N) {
  if(data && n == N)return;
  else if(data) {delete [] data; delete [] used;}

  n = N;
  data = new Contact[n * PARTICLE_MAX_CONTACTS];
  used = new size_t[n];
  memset(&used[0], 0, n * sizeof(size_t));
};

void 
Contacts::ResetUse() { 
  for (size_t i = 0; i < n;++i)used[i] = 0;
};

Contact& 
Contacts::UseContact(size_t index) {
  size_t available = used[index];
  used[index]++;
  return data[index * PARTICLE_MAX_CONTACTS + available];
}
  
Contact& 
Contact::GetContact(size_t index, size_t second){
  return data[index * PARTICLE_MAX_CONTACTS + second];
}

size_t 
Contacts::GetNumContacts(size_t index) const
{
  return used[index];
}

size_t 
Contacts::GetTotalNumContacts() const {
  size_t numContacts = 0;
  for(size_t x=0; x < n; ++x) numContacts += used[x];
  return numContacts;
}

JVR_NAMESPACE_CLOSE_SCOPE