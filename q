[1mdiff --git a/src/pbd/collision.cpp b/src/pbd/collision.cpp[m
[1mindex 1dbb72a..0b52356 100644[m
[1m--- a/src/pbd/collision.cpp[m
[1m+++ b/src/pbd/collision.cpp[m
[36m@@ -464,7 +464,7 @@[m [mSelfCollision::SelfCollision(Particles* particles, const pxr::SdfPath& path,[m
   , _grid(NULL)[m
   , _thickness(thickness)[m
 {[m
[31m-  _grid.Init(_particles->GetNumParticles(), &_particles->position[0], 2.f * _thickness);[m
[32m+[m[32m  _grid.Init(_particles->GetNumParticles(), &_particles->position[0], _thickness);[m
 }[m
 [m
 SelfCollision::~SelfCollision()[m
[36m@@ -515,14 +515,18 @@[m [mvoid SelfCollision::_ResetContacts(Particles* particles)[m
   memset(&_hits[0], 0, _hits.size() * sizeof(int));[m
 [m
   _contacts.clear();[m
[31m-  _contacts.reserve(numParticles*MAX_COLLIDE_PARTICLES);[m
[32m+[m[32m  _contacts.reserve(numParticles * PARTICLE_MAX_COLLIDE);[m
   _p2c.clear();[m
[31m-  _p2c.reserve(numParticles*MAX_COLLIDE_PARTICLES);[m
[32m+[m[32m  _p2c.reserve(numParticles * PARTICLE_MAX_COLLIDE);[m
   _c2p.clear();[m
[31m-  _c2p.reserve(numParticles*MAX_COLLIDE_PARTICLES);[m
[32m+[m[32m  _c2p.reserve(numParticles * PARTICLE_MAX_COLLIDE);[m
 [m
   _datas.resize(numParticles);[m
[31m-  for (_Contacts& contacts : _datas) contacts.clear();[m
[32m+[m[32m  _available.resize(numParticles);[m
[32m+[m[32m  for(size_t p=0; p<numParticles; ++p) {[m
[32m+[m[32m    _datas[p].resize(PARTICLE_MAX_COLLIDE);[m
[32m+[m[32m    _available[p] = 0;[m
[32m+[m[32m  }[m
 [m
 }[m
 [m
[36m@@ -546,12 +550,14 @@[m [mvoid SelfCollision::_FindContact(Particles* particles, size_t index, float ft)[m
     const pxr::GfVec3f otherPredicted(particles->position[closest] + particles->velocity[closest] * ft);[m
 [m
     if ((predicted - otherPredicted).GetLength() < (_thickness+TOLERANCE_MARGIN)) {[m
[31m-        Contact contact;[m
[32m+[m[32m        if (_available[index] >= _datas[index].size())[m
[32m+[m[32m          _datas[index].resize(_datas[index].size() + 8);[m
[32m+[m[32m        Contact* contact = &_datas[index][_available[index]++].data;[m
[32m+[m[41m        [m
         particles->color[index] = _grid.GetColor(particles->position[index]);[m
         particles->color[closest] = _grid.GetColor(particles->position[index]);[m
[31m-        _StoreContactLocation(particles, index, closest, &contact, ft);[m
[31m-        //_datas[index].push_back({closest, contact});[m
[31m-        intersect = false;[m
[32m+[m[32m        _StoreContactLocation(particles, index, closest, contact, ft);[m
[32m+[m[32m        intersect = true;[m
     }[m
   }[m
   SetHit(index, intersect);[m
[36m@@ -560,6 +566,7 @@[m [mvoid SelfCollision::_FindContact(Particles* particles, size_t index, float ft)[m
 [m
 void SelfCollision::_StoreContactLocation(Particles* particles, int index, int other, Contact* contact, float ft)[m
 {[m
[32m+[m[32m  return;[m
   const pxr::GfVec3f predicted(particles->position[index] + particles->velocity[index] * ft);[m
   const pxr::GfVec3f otherPredicted(particles->position[other] + particles->velocity[other] * ft);[m
 [m
[36m@@ -611,7 +618,7 @@[m [mvoid SelfCollision::_BuildContacts(Particles* particles, const std::vector<Body*[m
     _offsets[index] = contactsOffset;[m
     if(_counts[index]) {[m
       for(_Contact& contact: _datas[index]) {[m
[31m-        _contacts.push_back(contact.contact); [m
[32m+[m[32m        _contacts.push_back(contact.data);[m[41m [m
         _c2p.push_back(index); [m
         _ids.push_back(contact.id);[m
       };[m
[1mdiff --git a/src/pbd/collision.h b/src/pbd/collision.h[m
[1mindex b5cc0e5..11cada7 100644[m
[1m--- a/src/pbd/collision.h[m
[1m+++ b/src/pbd/collision.h[m
[36m@@ -23,27 +23,25 @@[m [mclass BVH;[m
 class HashGrid;[m
 [m
 [m
[31m-static const size_t PARTICLE_MAX_COLLIDE = 16;[m
[31m-[m
[31m-struct _Contact {[m
[31m-  int      id;[m
[31m-  Contact  contact;[m
[31m-[m
[31m-  _Contact() :id(Location::INVALID_INDEX) {};[m
[31m-  _Contact(int id) :id(id) {};[m
[31m-  _Contact(int id, const Contact& contact) :id(id), contact(contact) {};[m
[32m+[m[32mclass Collision : public Mask[m
[32m+[m[32m{[m
[32m+[m[32mpublic:[m
 [m
[31m-  _Contact(const _Contact& other) { id = other.id; contact = other.contact; };[m
[31m-};[m
[32m+[m[32m  static const size_t PARTICLE_MAX_COLLIDE = 16;[m
 [m
[31m-using _Contacts = std::vector<_Contact>;[m
[31m-using _ParticleContacts = std::vector<_Contacts>;[m
[32m+[m[32m  struct _Contact {[m
[32m+[m[32m    int      id;[m
[32m+[m[32m    Contact  data;[m
 [m
[32m+[m[32m    _Contact() : id(Location::INVALID_INDEX) {};[m
[32m+[m[32m    _Contact(int index) :id(index) {};[m
[32m+[m[32m    _Contact(int index, const Contact& contact) :id(index), data(contact) {};[m
 [m
[32m+[m[32m    _Contact(const _Contact& other) :id(other.id), data(other.data){};[m
[32m+[m[32m  };[m
 [m
[31m-class Collision : public Mask[m
[31m-{[m
[31m-public:[m
[32m+[m[32m  using _Contacts = std::vector<_Contact>;[m
[32m+[m[32m  using _ParticleContacts = std::vector<_Contacts>;[m
 [m
   enum Type {[m
     PLANE = 1,[m
[36m@@ -245,12 +243,12 @@[m [mprivate:[m
   HashGrid             _grid;[m
   float                _thickness;[m
   Particles*           _particles;[m
[31m-  std::vector<Contact> _contacts;[m
   std::vector<int>     _ids;                // contact component id[m
   std::vector<int>     _counts;             // per particle num neighbor[m
   std::vector<int>     _offsets;            // per particle neighbors access in flat list[m
[32m+[m[32m  std::vector<int>     _available;          // as we allocate blocks of contacts, track available one[m
   _ParticleContacts    _datas;              // per particle vector of contact filled in parallel[m
[31m-  std:vector<int>      _available;          // as we allocate blocks of contacts, track available one[m
[32m+[m
   [m
 };[m
 [m
[1mdiff --git a/src/tests/particles.cpp b/src/tests/particles.cpp[m
[1mindex 8d5ad74..bf28b3f 100644[m
[1m--- a/src/tests/particles.cpp[m
[1m+++ b/src/tests/particles.cpp[m
[36m@@ -202,7 +202,7 @@[m [mvoid TestParticles::InitExec(pxr::UsdStageRefPtr& stage)[m
   std::cout << "added ground" << std::endl;[m
 [m
  pxr::SdfPath selfCollideId = _solverId.AppendChild(pxr::TfToken("SelfCollision"));[m
[31m-  Collision* selfCollide = new SelfCollision(_solver->GetParticles(), selfCollideId, restitution, friction, radius * 0.5f );[m
[32m+[m[32m  Collision* selfCollide = new SelfCollision(_solver->GetParticles(), selfCollideId, restitution, friction, radius );[m
   _solver->AddElement(selfCollide, NULL, selfCollideId);[m
 [m
   std::cout << "added self collision constraint" << std::endl;[m
