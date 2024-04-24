#ifndef JVR_PBD_PARTICLE_H
#define JVR_PBD_PARTICLE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"
#include "../pbd/element.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

class Body : public Element
{

public:
  Body(Geometry* geom, size_t offset, size_t n, const pxr::GfVec3f& color,
    float mass=1.f, float radius=0.01f, float damping=0.1f)
  : Element(Element::BODY)
  , _geometry(geom)
  , _offset(offset)
  , _numPoints(n)
  , _mass(mass)
  , _radius(radius)
  , _damping(damping)
  , _color(color){}

  void SetOffset(size_t offset){_offset = offset;};
  void SetNumPoints(size_t numPoints){_numPoints = numPoints;};

  Geometry* GetGeometry() const {return _geometry;};
  size_t GetOffset() const {return _offset;};
  size_t GetNumPoints() const {return _numPoints;};
  float GetMass() const {return _mass;};
  float GetRadius() const {return _radius;};
  pxr::GfVec3f GetColor() const {return _color;};

protected:
  Geometry*     _geometry;
  size_t        _offset;
  size_t        _numPoints;
  float         _mass;
  float         _radius;
  float         _damping;
  pxr::GfVec3f  _color;
  //bool     _simulated;
};

enum BodyType
{
  RIGID,
  SOFT,
  CLOTH,
  HAIR
};

class Particles : public Element
{
public:
  enum State {MUTE, IDLE, ACTIVE};

  Particles() : Element(Element::PARTICLES){};

  size_t GetNumParticles() { return _position.size(); };
  void AddBody(Body* body, const pxr::GfMatrix4f& matrix);
  void RemoveBody(Body* body);
  void RemoveAllBodies();

  void SetAllState(short state);
  void SetBodyState(Body* body, short state);

  inline const short& State(size_t index) const{return _state[index];};
  inline const int& Body(size_t index) const{return _body[index];};
  inline const float& Mass(size_t index) const{return _mass[index];};
  inline const float& InvMass(size_t index) const{return _invMass[index];};
  inline const float& Radius(size_t index) const{return _radius[index];};
  inline const pxr::GfVec3f& Rest(size_t index) const{return _rest[index];};
  inline const pxr::GfVec3f& Previous(size_t index) const{return _previous[index];};
  inline const pxr::GfVec3f& Position(size_t index) const{return _position[index];};
  inline const pxr::GfVec3f& Predicted(size_t index) const{return _predicted[index];};
  inline const pxr::GfVec3f& Velocity(size_t index) const{return _velocity[index];};
  inline const pxr::GfVec3f& Color(size_t index) const{return _color[index];};

  inline short& State(size_t index) {return _state[index];};
  inline int& Body(size_t index) {return _body[index];};
  inline float& Mass(size_t index) {return _mass[index];};
  inline float& InvMass(size_t index) {return _invMass[index];};
  inline float& Radius(size_t index) {return _radius[index];};
  inline pxr::GfVec3f& Rest(size_t index) {return _rest[index];};
  inline pxr::GfVec3f& Previous(size_t index) {return _previous[index];};
  inline pxr::GfVec3f& Position(size_t index) {return _position[index];};
  inline pxr::GfVec3f& Predicted(size_t index) {return _predicted[index];};
  inline pxr::GfVec3f& Velocity(size_t index) {return _velocity[index];};
  inline pxr::GfVec3f& Color(size_t index) {return _color[index];};

  inline const auto& pxr::VtArray<short>& StateArray() const {return _state;};
  inline const auto& pxr::VtArray<int>& BodyArray() const {return _body;};
  inline const auto& pxr::VtArray<float>& MassArray() const {return _mass;};
  inline const auto& pxr::VtArray<float>& InvMassArray() const {return _invMasses;};
  inline const auto& pxr::VtArray<float>& RadiusArray() const {return _radius;};
  inline const auto& pxr::VtArray<pxr::GfVec3f>& RestArray() const {return _rest;};
  inline const auto& pxr::VtArray<pxr::GfVec3f>& PreviousArray() const {return _previous;};
  inline const auto& pxr::VtArray<pxr::GfVec3f>& PositionArray() const {return _position;};
  inline const auto& pxr::VtArray<pxr::GfVec3f>& PredictedArray() const {return _predicted;};
  inline const auto& pxr::VtArray<pxr::GfVec3f>& VelocityArray() const {return _velocity;};
  inline const auto& pxr::VtArray<pxr::GfVec3f>& ColorArray() const {return _color;};

  inline auto& pxr::VtArray<short>& StateArray() {return _state;};
  inline auto& pxr::VtArray<int>& BodyArray() {return _body;};
  inline auto& pxr::VtArray<float>& MassArray() {return _mass;};
  inline auto& pxr::VtArray<float>& InvMassArray() {return _invMasses;};
  inline auto& pxr::VtArray<float>& RadiusArray() {return _radius;};
  inline auto& pxr::VtArray<pxr::GfVec3f>& RestArray() {return _rest;};
  inline auto& pxr::VtArray<pxr::GfVec3f>& PreviousArray() {return _previous;};
  inline auto& pxr::VtArray<pxr::GfVec3f>& PositionArray() {return _position;};
  inline auto& pxr::VtArray<pxr::GfVec3f>& PredictedArray() {return _predicted;};
  inline auto& pxr::VtArray<pxr::GfVec3f>& VelocityArray() {return _velocity;};
  inline auto& pxr::VtArray<pxr::GfVec3f>& ColorArray() {return _color;};

private:
  pxr::VtArray<short>        _state;
  pxr::VtArray<int>          _body;
  pxr::VtArray<float>        _mass;
  pxr::VtArray<float>        _invMass;
  pxr::VtArray<float>        _radius;
  pxr::VtArray<pxr::GfVec3f> _rest;
  pxr::VtArray<pxr::GfVec3f> _previous;
  pxr::VtArray<pxr::GfVec3f> _position;
  pxr::VtArray<pxr::GfVec3f> _predicted;
  pxr::VtArray<pxr::GfVec3f> _velocity;
  pxr::VtArray<pxr::GfVec3f> _color;

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
