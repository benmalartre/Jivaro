#ifndef JVR_PBD_PARTICLE_H
#define JVR_PBD_PARTICLE_H

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Geometry;

struct Body
{
  float          damping;
  float          radius;
  float          mass;

  size_t         offset;
  size_t         numPoints;
  
  Geometry*      geometry;
};

enum BodyType
{
  RIGID,
  SOFT,
  CLOTH,
  HAIR
};

struct Particles
{
  size_t GetNumParticles() { return position.size(); };
  void AddBody(Body* body, const pxr::GfMatrix4f& matrix);
  void RemoveBody(Body* body);

  pxr::VtArray<int>          body;
  pxr::VtArray<float>        mass; // inverse mass
  pxr::VtArray<float>        radius;
  pxr::VtArray<pxr::GfVec3f> rest;
  pxr::VtArray<pxr::GfVec3f> position;
  pxr::VtArray<pxr::GfVec3f> predicted;
  pxr::VtArray<pxr::GfVec3f> velocity;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_PARTICLE_H
