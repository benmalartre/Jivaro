#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

PBDParticle::PBDParticle() 
  : _N(0)
  , _flip(false)
{
}

PBDParticle::~PBDParticle()
{
}

size_t PBDParticle::AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m)
{
  if (_geometries.find(geom) == _geometries.end()) {
    size_t base = _position[0].size();
    size_t add = geom->GetNumPoints();
  
    _geometries[geom] = PBDGeometry({base, m, m.GetInverse()});

    size_t newSize = base + add;
    _position[0].resize(newSize);
    _position[1].resize(newSize);
    _previous[0].resize(newSize);
    _previous[1].resize(newSize);
    _rest.resize(newSize);
    //_preload.resize(newSize);
    _force.resize(newSize);
    _mass.resize(newSize);

    for (size_t p = 0; p < geom->GetNumPoints(); ++p) {
      const pxr::GfVec3f pos = m.Transform(geom->GetPosition(p));
      size_t idx = base + p;
      _position[0][idx] = pos;
      _position[1][idx] = pos;
      _previous[0][idx] = pos;
      _previous[1][idx] = pos;
      _rest[idx] = pos;
      //_preload[idx] = pxr::GfVec3f(0.f);
      _force[idx] = pxr::GfVec3f(0.f);
      _mass[idx] = 1.f;
    }
    _N += add;
    return base;
  }
  return 0;
}

void PBDParticle::RemoveGeometry(Geometry* geom)
{
  if (_geometries.find(geom) != _geometries.end()) {
    PBDGeometry& desc = _geometries[geom];
    size_t base = desc.offset;
    size_t shift = geom->GetNumPoints();
    size_t remaining = _position[0].size() - (base + shift);

    for (size_t r = 0; r < remaining; ++r) {
      _position[0][base + r] = _position[0][base + shift +r];
      _position[1][base + r] = _position[1][base + shift + r];
      _previous[0][base + r] = _previous[0][base + shift + r];
      _previous[1][base + r] = _previous[1][base + shift + r];
      _rest[base + r] = _rest[base + shift + r];
      //_preload[base + r] = _preload[base + shift + r];
      _force[base + r] = _force[base + shift + r];
      _mass[base + r] = _mass[base + shift + r];
    }

    size_t newSize = _position[0].size() - shift;
    _position[0].resize(newSize);
    _position[1].resize(newSize);
    _previous[0].resize(newSize);
    _previous[1].resize(newSize);
    _rest.resize(newSize);
    //_preload.resize(newSize);
    _force.resize(newSize);
    _mass.resize(newSize);

    _geometries.erase(geom);
    _N -= shift;
  }
}

void PBDParticle::UpdateInput(Geometry* geom, 
    const pxr::VtArray<pxr::GfVec3f>& p, const pxr::GfMatrix4f& m)
{
  if (_geometries.find(geom) != _geometries.end()) {
    PBDGeometry& desc = _geometries[geom];
  }
}

void PBDParticle::Reset()
{
  for (size_t p = 0; p < _N; ++p) {
    _position[0][p] = _rest[p];
    _position[1][p] = _rest[p];
    _previous[0][p] = _rest[p];// -_preload[p];
    _previous[1][p] = _rest[p];// -_preload[p];
    _force[p] = pxr::GfVec3f(0.f);
  }
}

void PBDParticle::Integrate(float step)
{
  for(int i=0; i<_N; ++i) {
    pxr::GfVec3f& position = _position[_flip][i];
    pxr::GfVec3f tmp = position;
    pxr::GfVec3f& previous = _previous[_flip][i];
    position += position - previous + _force[i] * step * step;
    previous = tmp;
  }
}

void PBDParticle::AccumulateForces(const pxr::GfVec3f& gravity)
{
  for (int i = 0; i < _N; i++) {
    _force[i] = gravity;
  }
}

void PBDParticle::UpdateGeometries()
{
  std::map<Geometry*, PBDGeometry>::iterator it = _geometries.begin();
  for (; it != _geometries.end(); ++it)
  {
    Geometry* geom = it->first;
    size_t numPoints = geom->GetNumPoints();
    pxr::VtArray<pxr::GfVec3f> results(numPoints);
    for (size_t p = 0; p < numPoints; ++p) {
      results[p] = it->second.invMatrix.Transform(_position[_flip][it->second.offset + p]);
    }
    geom->SetPositions(&results[0], numPoints);
  }
}


JVR_NAMESPACE_CLOSE_SCOPE