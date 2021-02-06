#include "integration.h"

AMN_NAMESPACE_OPEN_SCOPE

// ----------------------------------------------------------------------------------------------
void PBDIntegration::SemiImplicitEuler(
  const float h, 
  const float mass, 
  pxr::GfVec3f& position,
  pxr::GfVec3f& velocity,
  const pxr::GfVec3f& acceleration)
{
  if (mass != 0.0)
  {
    velocity += acceleration * h;
    position += velocity * h;
  }
}

// ----------------------------------------------------------------------------------------------
void PBDIntegration::SemiImplicitEulerRotation(
  const float h,
  const float mass,
  const pxr::GfMatrix3f& invInertiaW,
  pxr::GfQuatf& rotation,
  pxr::GfVec3f& angularVelocity,	
  const pxr::GfVec3f& torque)
{
  if (mass != 0.0)
  {
    // simple form without nutation effect
    angularVelocity += h * invInertiaW * torque;

    pxr::GfQuatf angVelQ(0.0, angularVelocity[0], angularVelocity[1], angularVelocity[2]);
    rotation += h * 0.5 * (angVelQ * rotation);
    rotation.Normalize();
  }
}

// ----------------------------------------------------------------------------------------------
void PBDIntegration::VelocityUpdateFirstOrder(
	const float h,
	const float mass,
	const pxr::GfVec3f& position,
	const pxr::GfVec3f& oldPosition,
	pxr::GfVec3f& velocity)
{
	if (mass != 0.0)
		velocity = (1.0 / h) * (position - oldPosition);
}

// ----------------------------------------------------------------------------------------------
void PBDIntegration::AngularVelocityUpdateFirstOrder(
	const float h,
	const float mass,
	const pxr::GfQuatf& rotation,
	const pxr::GfQuatf& oldRotation,
	pxr::GfVec3f& angularVelocity)
{
	if (mass != 0.0)
	{
		const pxr::GfQuatf relRot = (rotation * oldRotation.GetConjugate());
		angularVelocity = relRot.GetImaginary() * (2.f / h);
	}
}

// ----------------------------------------------------------------------------------------------
void PBDIntegration::VelocityUpdateSecondOrder(
	const float h,
	const float mass,
	const pxr::GfVec3f& position,
	const pxr::GfVec3f& oldPosition,
	const pxr::GfVec3f& positionOfLastStep,
	pxr::GfVec3f& velocity)
{
	if (mass != 0.0)
		velocity = (1.0 / h) * (1.5 * position - 2.0 * oldPosition + 0.5 * positionOfLastStep);
}

// ----------------------------------------------------------------------------------------------
void PBDIntegration::AngularVelocityUpdateSecondOrder(
	const float h,
	const float mass,
	const pxr::GfQuatf& rotation,				
	const pxr::GfQuatf& oldRotation,			
	const pxr::GfQuatf& rotationOfLastStep,	
	pxr::GfVec3f& angularVelocity)
{
	// ToDo: is still first order
	if (mass != 0.0)
	{
		const pxr::GfQuatf relRot = (rotation * oldRotation.GetConjugate());
		angularVelocity = relRot.GetImaginary() *(2.0 / h);
	}
}

AMN_NAMESPACE_CLOSE_SCOPE