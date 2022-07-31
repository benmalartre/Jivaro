#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

PBDParticle::PBDParticle() 
  : _N(0)
{
}

PBDParticle::~PBDParticle()
{
}

void PBDParticle::AddGeometry(Geometry* geom)
{
  std::cout << "PBD PARTICLE : ADD GEOMETRY" << std::endl;
  if (_geometries.find(geom) == _geometries.end()) {
    size_t base = _position.size();
    size_t add = geom->GetNumPoints();
  
    _geometries[geom] = base;

    size_t newSize = base + add;
    _position.resize(newSize);
    _previous.resize(newSize);
    _force.resize(newSize);
    _mass.resize(newSize);

    for (size_t p = 0; p < geom->GetNumPoints(); ++p) {
      const pxr::GfVec3f& pos = geom->GetPosition(p);
      size_t idx = base + p;
      _position[idx] = pos;
      _previous[idx] = pos;
      _force[idx] = pxr::GfVec3f(0.f);
      _mass[idx] = 1.f;
    }
    _N += add;
  }
  std::cout << "N : " << _N << std::endl;
  std::cout << "S : " << _position.size() << std::endl;
}

void PBDParticle::RemoveGeometry(Geometry* geom)
{
  std::cout << "PBD PARTICLE : REMOVE GEOMETRY" << std::endl;
  if (_geometries.find(geom) != _geometries.end()) {
    size_t base = _geometries[geom];
    size_t shift = geom->GetNumPoints();
    size_t remaining = _position.size() - (base + shift);

    for (size_t r = 0; r < remaining; ++r) {
      _position[base + r] = _position[base + shift +r];
      _previous[base + r] = _previous[base + shift + r];
      _force[base + r] = _force[base + shift + r];
      _mass[base + r] = _mass[base + shift + r];
    }

    size_t newSize = _position.size() - shift;
    _position.resize(newSize);
    _previous.resize(newSize);
    _force.resize(newSize);
    _mass.resize(newSize);

    _geometries.erase(geom);
    _N -= shift;
  }
  std::cout << "N : " << _N << std::endl;
  std::cout << "S : " << _position.size() << std::endl;
}

void PBDParticle::Integrate(float step)
{
  for(int i=0; i<_N; ++i) {
    pxr::GfVec3f& position = _position[i];
    pxr::GfVec3f tmp = position;
    pxr::GfVec3f& previous = _previous[i];
    position += position - previous + _force[i] * step * step;
    previous = tmp;
  }
}

void PBDParticle::SatisfyConstraints()
{

}

void PBDParticle::AccumulateForces(const pxr::GfVec3f& gravity)
{
  for (int i = 0; i < _N; i++) {
    _force[i] = gravity;
  }
}

void PBDParticle::UpdateGeometries()
{
  for (std::map<Geometry*, size_t>::iterator it = _geometries.begin(); it != _geometries.end(); ++it)
  {
    Geometry* geom = it->first;
    geom->SetPositions(&_position[it->second], geom->GetNumPoints());
  }

}


JVR_NAMESPACE_CLOSE_SCOPE