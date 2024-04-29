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

class Contacts {

public:
  Contacts() : n(0), m(1), data(NULL){};
  virtual ~Contacts() { delete[] data; };

  Contact* GetPtr(size_t index=0) const {
    return &data[index];
  };
  Contact* GetPtr(size_t first, size_t second) const {
    return &data[first * m + second];
  };

  void Resize(size_t n, size_t m=PARTICLE_MAX_CONTACTS);
  void ResetUsed(size_t index);
  void ResetAllUsed();

  bool IsUsed(size_t index){return used[index] > 0;};

  Contact* Use(size_t index);
  Contact* Get(size_t index, size_t second=0);
  size_t GetNumUsed(size_t index) const;
  size_t GetTotalNumUsed() const;

private:
  size_t                n;
  size_t                m;
  std::vector<int>      used;
  Contact*              data;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_CONTACT_H
