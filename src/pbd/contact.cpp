#include "../geometry/geometry.h"
#include "../geometry/implicit.h"
#include "../geometry/deformable.h"
#include "../pbd/contact.h"
#include "../pbd/particle.h"
#include "../pbd/collision.h"


JVR_NAMESPACE_OPEN_SCOPE

void Contact::Init(const pxr::GfVec3f &normal, const pxr::GfVec3f &velocity, const float depth)
{
  _normal = normal;
  _velocity = velocity;
  _initDepth = depth;
  if(_initDepth > 0.f) _initDepth = 0.f;
}

void Contact::Update(const pxr::GfVec3f &normal, const pxr::GfVec3f &velocity, const float depth)
{
  _normal = normal;
  _velocity = velocity;
  _depth = depth;
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

Contact*
Contacts::LastUsed(size_t index){
  if(used[index] > 0)
    return &data[index * m + used[index] - 1];
  return nullptr;
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