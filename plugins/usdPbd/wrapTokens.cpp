//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// GENERATED FILE.  DO NOT EDIT.
#include "pxr/external/boost/python/class.hpp"
#include "usdPbd/tokens.h"

PXR_NAMESPACE_USING_DIRECTIVE

#define _ADD_TOKEN(cls, name) \
    cls.add_static_property(#name, +[]() { return UsdPbdTokens->name.GetString(); });

void wrapUsdPbdTokens()
{
    pxr_boost::python::class_<UsdPbdTokensType, pxr_boost::python::noncopyable>
        cls("Tokens", pxr_boost::python::no_init);
    _ADD_TOKEN(cls, colliders);
    _ADD_TOKEN(cls, pbdBodies);
    _ADD_TOKEN(cls, pbdColliders);
    _ADD_TOKEN(cls, pbdCollisionDamp);
    _ADD_TOKEN(cls, pbdCollisionEnabled);
    _ADD_TOKEN(cls, pbdCollisionStiffness);
    _ADD_TOKEN(cls, pbdConstraint);
    _ADD_TOKEN(cls, pbdConstraint_MultipleApplyTemplate_ConstraintEnabled);
    _ADD_TOKEN(cls, pbdConstraint_MultipleApplyTemplate_Damp);
    _ADD_TOKEN(cls, pbdConstraint_MultipleApplyTemplate_Stiffness);
    _ADD_TOKEN(cls, pbdDamp);
    _ADD_TOKEN(cls, pbdFriction);
    _ADD_TOKEN(cls, pbdGravity);
    _ADD_TOKEN(cls, pbdMargin);
    _ADD_TOKEN(cls, pbdMass);
    _ADD_TOKEN(cls, pbdMaxSeparationVelocity);
    _ADD_TOKEN(cls, pbdRadius);
    _ADD_TOKEN(cls, pbdRestitution);
    _ADD_TOKEN(cls, pbdSelfCollisionDamp);
    _ADD_TOKEN(cls, pbdSelfCollisionEnabled);
    _ADD_TOKEN(cls, pbdSelfCollisionFriction);
    _ADD_TOKEN(cls, pbdSelfCollisionMaxSeparationVelocity);
    _ADD_TOKEN(cls, pbdSelfCollisionRadius);
    _ADD_TOKEN(cls, pbdSelfCollisionRestitution);
    _ADD_TOKEN(cls, pbdShowConstraints);
    _ADD_TOKEN(cls, pbdShowPoints);
    _ADD_TOKEN(cls, pbdSimulationEnabled);
    _ADD_TOKEN(cls, pbdSleepThreshold);
    _ADD_TOKEN(cls, pbdStartFrame);
    _ADD_TOKEN(cls, pbdSubSteps);
    _ADD_TOKEN(cls, pbdVelocity);
    _ADD_TOKEN(cls, PbdBodyAPI);
    _ADD_TOKEN(cls, PbdCollisionAPI);
    _ADD_TOKEN(cls, PbdConstraintAPI);
    _ADD_TOKEN(cls, PbdSolver);
}
