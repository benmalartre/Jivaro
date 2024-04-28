#ifndef JVR_PBD_CONTACT_H
#define JVR_PBD_CONTACT_H

#include "../geometry/location.h"

JVR_NAMESPACE_OPEN_SCOPE

class Particles;
class Plane;
class Sphere;
class Mesh;
class Collision;
class Contact : public Location {
public:
  Contact(){};
  virtual ~Contact(){};

  void Init(Collision* collision, Particles* particles, size_t index, size_t other);
  void Update(Collision* collision, Particles* particles, size_t index);

  const pxr::GfVec3f& GetNormal() const {return _normal;};
  const pxr::GfVec3f& GetVelocity() const {return _velocity;};
  float GetSpeed() const { return _speed; };
  float GetDepth() const {return _d;};

private:
  pxr::GfVec3f      _normal;   // contact normal
  pxr::GfVec3f      _velocity; // relative velocity

  float             _d;        // penetration depth
  float             _speed;    // normal speed
};

static const size_t PARTICLE_MAX_CONTACTS = 16;

struct _Contacts {

  _Contacts() : n(0), data(NULL), used(NULL){};
  ~_Contacts(){delete [] data;};

  void Resize(size_t N) {
    if(data && n == N)return;
    else if(data) {delete [] data; delete [] used;}

    n = N;
    data = new Contact[n * PARTICLE_MAX_CONTACTS];
    used = new size_t[n];
    memset(&used[0], 0, n * sizeof(size_t));
  };

  Contact* operator [](int index) {
    return &data[index * PARTICLE_MAX_CONTACTS];
  };
  const Contact* operator [](int index) const {
    return &data[index * PARTICLE_MAX_CONTACTS];
  }; 

  void ResetUse() { 
    for (size_t i = 0; i < n;++i)used[i] = 0;
  };

  Contact& UseContact(size_t index) {
    size_t available = used[index];
    used[index]++;
    return data[index * PARTICLE_MAX_CONTACTS + available];
  }
    
  Contact& GetContact(size_t index, size_t second){
    return data[index * PARTICLE_MAX_CONTACTS + second];
  }

  size_t GetNumContacts(size_t index) {return used[index];}

  size_t GetTotalNumContacts() {
    size_t numContacts = 0;
    for(size_t x=0; x < n; ++x) numContacts += used[x];
    return numContacts;
  }

  size_t    n;
  size_t*   used;
  Contact*  data;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_CONTACT_H
