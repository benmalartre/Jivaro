#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

PBDParticle::PBDParticle() 
{
}

PBDParticle::~PBDParticle()
{
}

size_t PBDParticle::AddGeometry(PBDGeometry* geom)
{
  std::cout << "[system] add geometry : " << geom << std::endl;
  size_t base = _position.size();
  size_t add = geom->ptr->GetNumPoints();
  
  size_t newSize = base + add;
  _position.resize(newSize);
  _previous.resize(newSize);
  _rest.resize(newSize);
  _preload.resize(newSize);
  _force.resize(newSize);
  _mass.resize(newSize);

  const pxr::GfVec3f* points = geom->ptr->GetPositionsCPtr();
  for (size_t p = 0; p < add; ++p) {
    const pxr::GfVec3f pos = geom->matrix.Transform(points[p]);
    size_t idx = base + p;
    _position[idx] = pos;
    _previous[idx] = pos;
    _rest[idx] = pos;
    _preload[idx] = pxr::GfVec3f(0.f);
    _force[idx] = pxr::GfVec3f(0.f);
    _mass[idx] = 1.f;
  }
  return base;
}

void PBDParticle::RemoveGeometry(PBDGeometry* geom)
{
  std::cout << "[system] remove geometry : " << geom << std::endl;
  size_t base = geom->offset;
  size_t shift = geom->ptr->GetNumPoints();
  size_t remaining = _position.size() - (base + shift);

  for (size_t r = 0; r < remaining; ++r) {
    _position[0][base + r] = _position[0][base + shift +r];
    _position[1][base + r] = _position[1][base + shift + r];
    _previous[0][base + r] = _previous[0][base + shift + r];
    _previous[1][base + r] = _previous[1][base + shift + r];
    _rest[base + r] = _rest[base + shift + r];
    _preload[base + r] = _preload[base + shift + r];
    _force[base + r] = _force[base + shift + r];
    _mass[base + r] = _mass[base + shift + r];
  }

  size_t newSize = _position.size() - shift;
  _position.resize(newSize);
  _previous.resize(newSize);
  _rest.resize(newSize);
  _preload.resize(newSize);
  _force.resize(newSize);
  _mass.resize(newSize);

}

void PBDParticle::Reset(size_t startIdx, size_t endIdx)
{
  for (size_t p = startIdx; p < endIdx; ++p) {
    _position[p] = _rest[p];
    _position[p] = _rest[p];
    _previous[p] = _rest[p] -_preload[p];
    _previous[p] = _rest[p] -_preload[p];
    _force[p] = pxr::GfVec3f(0.f);
  }
}

void PBDParticle::Integrate(size_t startIdx, size_t endIdx, float step)
{
  for(int i = startIdx; i < endIdx; ++i) {
    pxr::GfVec3f& position = _position[i];
    pxr::GfVec3f tmp = position;
    pxr::GfVec3f& previous = _previous[i];
    position += position - previous + _force[i] * step * step;
    previous = tmp;
  }
}

void PBDParticle::AccumulateForces(size_t startIdx, size_t endIdx, const pxr::GfVec3f& gravity)
{
  size_t numParticles = GetNumParticles();
  for (int i = startIdx; i < endIdx; ++i) {
    _force[i] = gravity;
  }
}



JVR_NAMESPACE_CLOSE_SCOPE