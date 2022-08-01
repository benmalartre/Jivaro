#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

PBDParticle::PBDParticle() 
  : _N(0)
{
}

PBDParticle::~PBDParticle()
{
}

void PBDParticle::AddGeometry(Geometry* geom, const pxr::GfMatrix4f& m)
{
  if (_geometries.find(geom) == _geometries.end()) {
    size_t base = _position.size();
    size_t add = geom->GetNumPoints();
  
    _geometries[geom] = PBDGeometry({base, m, m.GetInverse()});

    size_t newSize = base + add;
    _position.resize(newSize);
    _previous.resize(newSize);
    _initial.resize(newSize);
    _preload.resize(newSize);
    _force.resize(newSize);
    _mass.resize(newSize);

    for (size_t p = 0; p < geom->GetNumPoints(); ++p) {
      const pxr::GfVec3f pos = m.Transform(geom->GetPosition(p));
      size_t idx = base + p;
      _position[idx] = pos;
      _previous[idx] = pos;
      _initial[idx] = pos;
      _preload[idx] = pxr::GfVec3f(0.f);
      _force[idx] = pxr::GfVec3f(0.f);
      _mass[idx] = 1.f;
    }
    _N += add;
  }
}

void PBDParticle::RemoveGeometry(Geometry* geom)
{
  if (_geometries.find(geom) != _geometries.end()) {
    PBDGeometry& desc = _geometries[geom];
    size_t base = desc.offset;
    size_t shift = geom->GetNumPoints();
    size_t remaining = _position.size() - (base + shift);

    for (size_t r = 0; r < remaining; ++r) {
      _position[base + r] = _position[base + shift +r];
      _previous[base + r] = _previous[base + shift + r];
      _initial[base + r] = _initial[base + shift + r];
      _preload[base + r] = _preload[base + shift + r];
      _force[base + r] = _force[base + shift + r];
      _mass[base + r] = _mass[base + shift + r];
    }

    size_t newSize = _position.size() - shift;
    _position.resize(newSize);
    _previous.resize(newSize);
    _initial.resize(newSize);
    _preload.resize(newSize);
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
    _position[p] = _initial[p];
    _previous[p] = _initial[p] - _preload[p];
    _force[p] = pxr::GfVec3f(0.f);
  }
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
  for(int i=0; i<_N; i++) { 

    //_position[i][0] = pxr::GfMin(pxr::GfMax(_position[i][0], -100.f), 100.f); 
    _position[i][1] = pxr::GfMax(_position[i][1], 0.f);
    //_position[i][2] = pxr::GfMin(pxr::GfMax(_position[i][2], 100.f), 100.f);
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
      results[p] = it->second.invMatrix.Transform(_position[it->second.offset + p]);
    }
    //geom->SetPositions(, geom->GetNumPoints());
  }

}


JVR_NAMESPACE_CLOSE_SCOPE