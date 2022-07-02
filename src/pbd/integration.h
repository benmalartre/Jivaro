#ifndef JVR_PBD_INTEGRATION_H
#define JVR_PBD_INTEGRATION_H

#include "../common.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/matrix3f.h>

JVR_NAMESPACE_OPEN_SCOPE

class PBDIntegration
{
public:	
  // Perform an integration step for a particle using the semi-implicit Euler method
  static void SemiImplicitEuler(
    const float h,
    const float mass,
    pxr::GfVec3f& position,
    pxr::GfVec3f& velocity,
    const pxr::GfVec3f& acceleration);

  // Semi-implicit Euler (symplectic Euler) for rotational part of a rigid body
  static void SemiImplicitEulerRotation(
    const float h,
    const float mass,
    const pxr::GfMatrix3f& invInertiaW,
    pxr::GfQuatf& rotation,
    pxr::GfVec3f& angularVelocity,
    const pxr::GfVec3f& torque);


  // Velocity update (first order)		
  static void VelocityUpdateFirstOrder(
    const float h,
    const float mass,
    const pxr::GfVec3f& position,           // position after constraint projection	at time t+h
    const pxr::GfVec3f& oldPosition,        // position before constraint projection at time t
    pxr::GfVec3f& velocity);

  // Angular velocity update (first order)
  static void AngularVelocityUpdateFirstOrder(
    const float h,
    const float mass,
    const pxr::GfQuatf& rotation,       // rotation after constraint projection	at time t+h
    const pxr::GfQuatf& oldRotation,    // rotation before constraint projection at time t
    pxr::GfVec3f& angularVelocity);


  // Velocity update (second order)
  static void VelocityUpdateSecondOrder(
    const float h,
    const float mass,
    const pxr::GfVec3f& position,           // position after constraint projection	at time t+h
    const pxr::GfVec3f& oldPosition,        // position before constraint projection at time t
    const pxr::GfVec3f& positionOfLastStep, // position of last simulation step at time t-h
    pxr::GfVec3f& velocity);

  // Angular velocity update (second order) 
  static void AngularVelocityUpdateSecondOrder(
    const float h,
    const float mass,
    const pxr::GfQuatf& rotation,           // rotation after constraint projection	at time t+h
    const pxr::GfQuatf& oldRotation,        // rotation before constraint projection at time t
    const pxr::GfQuatf& rotationOfLastStep, // rotation of last simulation step at time t-h
    pxr::GfVec3f& angularVelocity);

};

JVR_NAMESPACE_CLOSE_SCOPE

#endif