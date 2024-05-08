#ifndef JVR_PBD_CONTACT_H
#define JVR_PBD_CONTACT_H

#include "../geometry/location.h"

JVR_NAMESPACE_OPEN_SCOPE

class Particles;
class Plane;
class Sphere;
class Mesh;
class Collision;
class SelfCollision;


class Contact : public Location {
public:
  Contact(){};
  virtual ~Contact(){};

  void Init(Collision* collision, Particles* particles, size_t index);
  void Init(SelfCollision* collision, Particles* particles, size_t index, size_t other);
  void Update(Collision* collision, Particles* particles, size_t index);
  void Update(SelfCollision* collision, Particles* particles, size_t index, size_t other);

  const pxr::GfVec3f& GetNormal() const {return _normal;};
  const pxr::GfVec3f& GetVelocity() const {return _velocity;};
  float GetDepth() const {return _depth;};
  float GetInitDepth() const {return _initDepth;};

  bool GetHit() const {return _hit;};
  void SetHit(bool hit) {_hit=hit;};

private:
  pxr::GfVec3f      _normal;   // contact normal
  pxr::GfVec3f      _velocity; // relative velocity

  float             _initDepth;// start frame penetration depth
  float             _depth;    // current substep penetration depth

  bool              _hit;      // is penetrating 
};


static const size_t PARTICLE_MAX_CONTACTS = 16;

class Contacts {

public:
  Contacts() : n(0), m(1), data(NULL), used(NULL){};
  virtual ~Contacts() { delete[] data; delete[] used;};

  Contact* Get(size_t index, size_t second=0) const {
    return &data[index * m + second];
  };

  void Resize(size_t n, size_t m=PARTICLE_MAX_CONTACTS);
  void ResetUsed(size_t index);
  void ResetAllUsed();

  bool IsUsed(size_t index){return used[index] > 0;};

  Contact* Use(size_t index);
  size_t GetNumUsed(size_t index) const;
  size_t GetTotalNumUsed() const;


private:
  size_t                n;
  size_t                m;
  int*                  used;
  Contact*              data;
  std::mutex            mutex;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_CONTACT_H
