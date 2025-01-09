//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdPbd/tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

UsdPbdTokensType::UsdPbdTokensType() :
    colliders("colliders", TfToken::Immortal),
    pbdBodies("pbd:bodies", TfToken::Immortal),
    pbdColliders("pbd:colliders", TfToken::Immortal),
    pbdCollisionDamp("pbd:collisionDamp", TfToken::Immortal),
    pbdCollisionEnabled("pbd:collisionEnabled", TfToken::Immortal),
    pbdCollisionStiffness("pbd:collisionStiffness", TfToken::Immortal),
    pbdConstraint("pbd:constraint", TfToken::Immortal),
    pbdConstraint_MultipleApplyTemplate_ConstraintEnabled("pbd:constraint:__INSTANCE_NAME__:constraintEnabled", TfToken::Immortal),
    pbdConstraint_MultipleApplyTemplate_Damp("pbd:constraint:__INSTANCE_NAME__:damp", TfToken::Immortal),
    pbdConstraint_MultipleApplyTemplate_Stiffness("pbd:constraint:__INSTANCE_NAME__:stiffness", TfToken::Immortal),
    pbdDamp("pbd:damp", TfToken::Immortal),
    pbdFriction("pbd:friction", TfToken::Immortal),
    pbdGravity("pbd:gravity", TfToken::Immortal),
    pbdMargin("pbd:margin", TfToken::Immortal),
    pbdMass("pbd:mass", TfToken::Immortal),
    pbdMaxSeparationVelocity("pbd:maxSeparationVelocity", TfToken::Immortal),
    pbdRadius("pbd:radius", TfToken::Immortal),
    pbdRestitution("pbd:restitution", TfToken::Immortal),
    pbdSelfCollisionDamp("pbd:selfCollisionDamp", TfToken::Immortal),
    pbdSelfCollisionEnabled("pbd:selfCollisionEnabled", TfToken::Immortal),
    pbdSelfCollisionFriction("pbd:selfCollisionFriction", TfToken::Immortal),
    pbdSelfCollisionMaxSeparationVelocity("pbd:selfCollisionMaxSeparationVelocity", TfToken::Immortal),
    pbdSelfCollisionRadius("pbd:selfCollisionRadius", TfToken::Immortal),
    pbdSelfCollisionRestitution("pbd:selfCollisionRestitution", TfToken::Immortal),
    pbdShowConstraints("pbd:showConstraints", TfToken::Immortal),
    pbdShowPoints("pbd:showPoints", TfToken::Immortal),
    pbdSimulationEnabled("pbd:simulationEnabled", TfToken::Immortal),
    pbdSleepThreshold("pbd:sleepThreshold", TfToken::Immortal),
    pbdStartFrame("pbd:startFrame", TfToken::Immortal),
    pbdSubSteps("pbd:subSteps", TfToken::Immortal),
    pbdVelocity("pbd:velocity", TfToken::Immortal),
    PbdBodyAPI("PbdBodyAPI", TfToken::Immortal),
    PbdCollisionAPI("PbdCollisionAPI", TfToken::Immortal),
    PbdConstraintAPI("PbdConstraintAPI", TfToken::Immortal),
    PbdSolver("PbdSolver", TfToken::Immortal),
    allTokens({
        colliders,
        pbdBodies,
        pbdColliders,
        pbdCollisionDamp,
        pbdCollisionEnabled,
        pbdCollisionStiffness,
        pbdConstraint,
        pbdConstraint_MultipleApplyTemplate_ConstraintEnabled,
        pbdConstraint_MultipleApplyTemplate_Damp,
        pbdConstraint_MultipleApplyTemplate_Stiffness,
        pbdDamp,
        pbdFriction,
        pbdGravity,
        pbdMargin,
        pbdMass,
        pbdMaxSeparationVelocity,
        pbdRadius,
        pbdRestitution,
        pbdSelfCollisionDamp,
        pbdSelfCollisionEnabled,
        pbdSelfCollisionFriction,
        pbdSelfCollisionMaxSeparationVelocity,
        pbdSelfCollisionRadius,
        pbdSelfCollisionRestitution,
        pbdShowConstraints,
        pbdShowPoints,
        pbdSimulationEnabled,
        pbdSleepThreshold,
        pbdStartFrame,
        pbdSubSteps,
        pbdVelocity,
        PbdBodyAPI,
        PbdCollisionAPI,
        PbdConstraintAPI,
        PbdSolver
    })
{
}

TfStaticData<UsdPbdTokensType> UsdPbdTokens;

PXR_NAMESPACE_CLOSE_SCOPE
