#include "../geometry/geometry.h"
#include "../geometry/points.h"
#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

void Particles::AddBody(Body* body, const pxr::GfMatrix4f& matrix)
{
  Geometry* geom = body->GetGeometry();
  if(geom->GetType() < Geometry::POINT) return;

  size_t base = _position.size();
  size_t numPoints = geom->GetNumPoints();
  size_t size = base + numPoints;
  size_t index = _body.size() ? _body.back() + 1 : 0;
  float w = pxr::GfIsClose(body->GetMass(), 0.f, 0.000001f) ? 0.f : 1.f / body->GetMass();
  _mass.resize(size);
  _invMass.resize(size);
  _radius.resize(size);
  _rest.resize(size);
  _previous.resize(size);
  _position.resize(size);
  _predicted.resize(size);
  _velocity.resize(size);
  _body.resize(size);
  _color.resize(size);
  _state.resize(size);


  const pxr::VtArray<pxr::GfVec3f>& points = ((Points*)geom)->GetPositions();
  pxr::GfVec3f pos;
  size_t idx;
  for (size_t p = 0; p < numPoints; ++p) {
    pos = matrix.Transform(points[p]);
    idx = base + p;
    _mass[idx] = body->GetMass();
    _invMass[idx] = w;
    _radius[idx] = body->GetRadius();
    _rest[idx] = pos;
    _previous[idx] = pos;
    _position[idx] = pos;
    _predicted[idx] = pos;
    _velocity[idx] = pxr::GfVec3f(0.f);
    _body[idx] = index;
    _color[idx] = body->GetColor();
    _state[idx] = ACTIVE;
  }

  body->SetOffset(base);
  body->SetNumPoints(numPoints);
}

void Particles::RemoveBody(Body* b) 
{
  const size_t base = b->GetOffset();
  const size_t shift = b->GetNumPoints();
  const size_t remaining = GetNumParticles() - (base + shift);
  size_t lhi, rhi;

  for (size_t r = 0; r < remaining; ++r) {
    lhi = base + r;
    rhi = base + shift + r;
    _mass[lhi]      = _mass[rhi];
    _invMass[lhi]   = _invMass[rhi];
    _radius[lhi]    = _radius[rhi];
    _rest[lhi]      = _rest[rhi];
    _previous[lhi]  = _previous[rhi];
    _position[lhi]  = _position[rhi];
    _predicted[lhi] = _predicted[rhi];
    _velocity[lhi]  = _velocity[rhi];
    _body[lhi]      = _body[rhi] - 1;
    _color[lhi]     = _color[rhi];
    _state[lhi]     = _state[rhi];
  }

  size_t size = _position.size() - shift;
  _mass.resize(size);
  _invMass.resize(size);
  _radius.resize(size);
  _rest.resize(size);
  _previous.resize(size);
  _position.resize(size);
  _predicted.resize(size);
  _velocity.resize(size);
  _body.resize(size);
  _color.resize(size);
  _state.resize(size);
}

void Particles::RemoveAllBodies()
{
  _mass.clear();
  _invMass.clear();
  _radius.clear();
  _rest.clear();
  _previous.clear();
  _position.clear();
  _predicted.clear();
  _velocity.clear();
  _body.clear();
  _color.clear();
  _state.clear();
}

void Particles::SetAllState( short s)
{
  for(auto& state: _state) state = s;
}

void Particles::SetBodyState(Body* b, short s)
{
  const size_t begin = b->GetOffset();
  const size_t end = begin +b->GetNumPoints();

  for (size_t r = begin; r < end; ++r) {
    _state[r] = s;
  }
}


JVR_NAMESPACE_CLOSE_SCOPE