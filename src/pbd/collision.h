#ifndef JVR_PBD_COLLISION_H
#define JVR_PBD_COLLISION_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../acceleration/hashGrid.h"
#include "../pbd/contact.h"
#include "../pbd/mask.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

struct Particles;
struct Body;
class Constraint;
class CollisionConstraint;
class Points;
class Solver;
class BVH;
class HashGrid;


class Collision : public Mask
{
public:

  static const float TOLERANCE_MARGIN;

  enum Type {
    PLANE = 1,
    SPHERE,
    MESH,
    SELF
  };

  Collision(Geometry* collider, const pxr::SdfPath& path, 
    float restitution=0.5f, float friction=0.5f) 
    : Mask(Element::COLLISION)
    , _id(path)
    , _collider(collider)
    , _restitution(restitution)
    , _friction(friction) {};

  /*
  void AddBody(Particles* particles, Body* body);
  void RemoveBody(Particles* particles, Body* body);
  */
  virtual ~Collision() {};
  virtual size_t GetTypeId() const override = 0; // pure virtual

  virtual void Init(size_t numParticles);
  virtual void Update(const pxr::UsdPrim& prim, double time);
  virtual void FindContacts(Particles* particles, const std::vector<Body*>& bodies,
    std::vector<Constraint*>& constraints, float ft);
  virtual void StoreContactsLocation(Particles* particles, int* elements, size_t n, 
    float ft, bool solveInitialPenetration);
  virtual void UpdateContacts(Particles* particles, size_t begin, size_t end);


  virtual Geometry* GetGeometry(){return _collider;};

  virtual size_t GetContactComponent(size_t index, size_t c=0) const;
  virtual pxr::GfVec3f GetContactPosition(size_t index, size_t c=0) const;
  virtual pxr::GfVec3f GetContactNormal(size_t index, size_t c=0) const;
  virtual pxr::GfVec3f GetContactVelocity(size_t index, size_t c=0) const;
  virtual float GetContactDepth(size_t index, size_t c=0) const;
  virtual float GetContactInitDepth(size_t index, size_t c=0) const;

  Contacts& GetContacts(){return _contacts;};
  size_t GetNumContacts(size_t index){return _contacts.GetNumUsed(index);};
  size_t GetTotalNumContacts(){return _contacts.GetTotalNumUsed();};
  const std::vector<int>& GetC2P(){return _c2p;};

  virtual void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results,
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors);

  virtual float GetValue(Particles* particles, size_t index) = 0;
  virtual pxr::GfVec3f GetGradient(Particles* particles, size_t index) = 0; // pure virtual
  virtual pxr::GfVec3f GetVelocity(Particles* particles, size_t index);

  inline bool CheckHit(size_t index) {
    return BIT_CHECK(_hits[index/Mask::INT_BITS], index%Mask::INT_BITS);
  };
  inline void SetHit(size_t index, bool hit) {
    if(hit) BIT_SET(_hits[index/Mask::INT_BITS], index%Mask::INT_BITS);
    else BIT_CLEAR(_hits[index/Mask::INT_BITS], index%Mask::INT_BITS);
  };

  float GetFriction() const {return _friction;};
  float GetRestitution() const {return _restitution;};

protected:

  static const size_t PACKET_SIZE;

  virtual void _UpdateParameters(const pxr::UsdPrim& prim, double time);
  virtual void _ResetContacts(Particles* particles);
  virtual void _BuildContacts(Particles* particles, const std::vector<Body*>& bodies,
    std::vector<Constraint*>& constraints, float dt);
  virtual void _FindContacts(Particles* particles, size_t begin, size_t end, float ft);
  
  virtual void _FindContact(Particles* particles, size_t index, float ft) = 0; // pure virtual
  virtual void _StoreContactLocation(Particles* particles, int elem, Contact* contact, float ft){};
  virtual void _ResolveInitialPenetration(Particles* particles, size_t index);

  // hits encode vertex hit in the int list bits
  pxr::VtArray<int>                 _hits;
  std::vector<int>                  _c2p;
  size_t                            _numParticles;
  Contacts                          _contacts;   
  float                             _restitution;
  float                             _friction;
  Geometry*                         _collider;
  pxr::TfToken                      _key;

  Points*                           _points;
  pxr::SdfPath                      _id;

};

class PlaneCollision : public Collision
{
public:
  PlaneCollision(Geometry* collider, const pxr::SdfPath& path, 
    float restitution=0.5f, float friction= 0.5f);
  size_t GetTypeId() const override { return TYPE_ID; };

  float GetValue(Particles* particles, size_t index) override;
  pxr::GfVec3f GetGradient(Particles* particles, size_t index) override;
  void Update(const pxr::UsdPrim& prim, double time) override;

protected:
  void _UpdatePositionAndNormal();
  void _FindContact(Particles* particles, size_t index, float ft) override;
  void _StoreContactLocation(Particles* particles, int elem, Contact* contact, float ft) override;

private:
  static size_t                 TYPE_ID;
  pxr::GfVec3f                  _position;
  pxr::GfVec3f                  _normal;

};

class SphereCollision : public Collision
{
public:
  SphereCollision(Geometry* collider, const pxr::SdfPath& path, 
    float restitution=0.5f, float friction= 0.5f);
  size_t GetTypeId() const override { return TYPE_ID; };

  float GetValue(Particles* particles, size_t index) override;
  pxr::GfVec3f GetGradient(Particles* particles, size_t index) override;
  void Update(const pxr::UsdPrim& prim, double time) override;
  
protected:
  void _UpdateCenterAndRadius();
  void _FindContact(Particles* particles, size_t index, float ft) override;
  void _StoreContactLocation(Particles* particles, int elem, Contact* contact, float ft) override;
  

private:
  static size_t                 TYPE_ID;
  pxr::GfVec3f                  _center;
  float                         _radius;
};


class MeshCollision : public Collision
{
public:
  MeshCollision(Geometry* collider, const pxr::SdfPath& path, 
    float restitution=0.5f, float friction= 0.5f);
  size_t GetTypeId() const override { return TYPE_ID; };

  virtual void Init(size_t numParticles) override;
  const Location* GetQuery(size_t index){return &_query[index];};

  float GetValue(Particles* particles, size_t index) override;
  pxr::GfVec3f GetGradient(Particles* particles, size_t index) override;
  void Update(const pxr::UsdPrim& prim, double time) override;

  void GetPoints(Particles* particles, pxr::VtArray<pxr::GfVec3f>& results,
    pxr::VtArray<float>& radius, pxr::VtArray<pxr::GfVec3f>& colors) override;

protected:
  void _CreateAccelerationStructure();
  void _UpdateAccelerationStructure();
  void _FindContact(Particles* particles, size_t index, float ft) override;
  void _StoreContactLocation(Particles* particles, int elem, Contact* contact, float ft) override;
  

private:
  static size_t                 TYPE_ID;
  BVH*                          _bvh;
  std::vector<Location>         _query;
  std::vector<Location>         _closest;
};

class SelfCollision : public Collision
{

public:

  SelfCollision(Particles* particles, const pxr::SdfPath& path,  
    float restitution=0.5f, float friction= 0.5f);
  ~SelfCollision();
  size_t GetTypeId() const override { return TYPE_ID; };

  float GetValue(Particles* particles, size_t index) override{return 0.f;};
  pxr::GfVec3f GetGradient(Particles* particles, size_t index) override{return pxr::GfVec3f(0.f);};
  float GetValue(Particles* particles, size_t index, size_t other);;
  pxr::GfVec3f GetGradient(Particles* particles, size_t index, size_t other);
  pxr::GfVec3f GetVelocity(Particles* particles, size_t index, size_t other);

  void UpdateContacts(Particles* particles, size_t begin , size_t end) override;

  void Update(const pxr::UsdPrim& prim, double time) override;

  void FindContacts(Particles* particles, const std::vector<Body*>& bodies, 
    std::vector<Constraint*>& constraints, float ft)override;

protected:
  void _ComputeNeighbors(const std::vector<Body*>& bodies);
  void _UpdateAccelerationStructure();
  void _ResetContacts(Particles* particles) override;
  void _FindContacts(Particles* particles, size_t begin, size_t end, float ft) override;
  void _FindContact(Particles* particles, size_t index, float ft) override;
  void _StoreContactLocation(Particles* particles, int index, int other, Contact* contact, float ft);

  void _BuildContacts(Particles* particles, const std::vector<Body*>& bodies,
    std::vector<Constraint*>& constraints, float ft)override;

  inline bool _AreConnected(size_t lhs, size_t rhs);

private:
  static size_t                     TYPE_ID;
  HashGrid                          _grid;
  Particles*                        _particles;

  bool                              _neighborsInitialized;
  std::vector<int>                  _neighbors;
  std::vector<short>                _neighborsCounts;
  std::vector<int>                  _neighborsOffsets;

  
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_COLLISION_H
