//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDPBD_TOKENS_H
#define USDPBD_TOKENS_H

/// \file UsdPbd/tokens.h

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// This is an automatically generated file (by usdGenSchema.py).
// Do not hand-edit!
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "pxr/pxr.h"
#include "usdPbd/api.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/tf/token.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE


/// \class UsdPbdTokensType
///
/// \link UsdPbdTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// These tokens are auto-generated from the module's schema, representing
/// property names, for when you need to fetch an attribute or relationship
/// directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
/// manner, and allow the compiler to verify that you spelled the name
/// correctly.
///
/// UsdPbdTokens also contains all of the \em allowedTokens values
/// declared for schema builtin attributes of 'token' scene description type.
/// Use UsdPbdTokens like so:
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(UsdPbdTokens->colliders);
/// \endcode
struct UsdPbdTokensType {
    USDPBD_API UsdPbdTokensType();
    /// \brief "colliders"
    /// 
    /// collision objects for the solver. 
    const TfToken colliders;
    /// \brief "pbd:bodies"
    /// 
    /// UsdPbdSolver
    const TfToken pbdBodies;
    /// \brief "pbd:colliders"
    /// 
    /// UsdPbdSolver
    const TfToken pbdColliders;
    /// \brief "pbd:collisionDamp"
    /// 
    /// UsdPbdCollisionAPI
    const TfToken pbdCollisionDamp;
    /// \brief "pbd:collisionEnabled"
    /// 
    /// UsdPbdCollisionAPI
    const TfToken pbdCollisionEnabled;
    /// \brief "pbd:collisionStiffness"
    /// 
    /// UsdPbdCollisionAPI
    const TfToken pbdCollisionStiffness;
    /// \brief "pbd:constraint"
    /// 
    /// Property namespace prefix for the UsdPbdConstraintAPI schema.
    const TfToken pbdConstraint;
    /// \brief "pbd:constraint:__INSTANCE_NAME__:constraintEnabled"
    /// 
    /// UsdPbdConstraintAPI
    const TfToken pbdConstraint_MultipleApplyTemplate_ConstraintEnabled;
    /// \brief "pbd:constraint:__INSTANCE_NAME__:damp"
    /// 
    /// UsdPbdConstraintAPI
    const TfToken pbdConstraint_MultipleApplyTemplate_Damp;
    /// \brief "pbd:constraint:__INSTANCE_NAME__:stiffness"
    /// 
    /// UsdPbdConstraintAPI
    const TfToken pbdConstraint_MultipleApplyTemplate_Stiffness;
    /// \brief "pbd:damp"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdDamp;
    /// \brief "pbd:friction"
    /// 
    /// UsdPbdCollisionAPI
    const TfToken pbdFriction;
    /// \brief "pbd:gravity"
    /// 
    /// UsdPbdSolver
    const TfToken pbdGravity;
    /// \brief "pbd:margin"
    /// 
    /// UsdPbdCollisionAPI
    const TfToken pbdMargin;
    /// \brief "pbd:mass"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdMass;
    /// \brief "pbd:maxSeparationVelocity"
    /// 
    /// UsdPbdCollisionAPI
    const TfToken pbdMaxSeparationVelocity;
    /// \brief "pbd:radius"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdRadius;
    /// \brief "pbd:restitution"
    /// 
    /// UsdPbdCollisionAPI
    const TfToken pbdRestitution;
    /// \brief "pbd:selfCollisionDamp"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdSelfCollisionDamp;
    /// \brief "pbd:selfCollisionEnabled"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdSelfCollisionEnabled;
    /// \brief "pbd:selfCollisionFriction"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdSelfCollisionFriction;
    /// \brief "pbd:selfCollisionMaxSeparationVelocity"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdSelfCollisionMaxSeparationVelocity;
    /// \brief "pbd:selfCollisionRadius"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdSelfCollisionRadius;
    /// \brief "pbd:selfCollisionRestitution"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdSelfCollisionRestitution;
    /// \brief "pbd:showConstraints"
    /// 
    /// UsdPbdSolver
    const TfToken pbdShowConstraints;
    /// \brief "pbd:showPoints"
    /// 
    /// UsdPbdSolver
    const TfToken pbdShowPoints;
    /// \brief "pbd:simulationEnabled"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdSimulationEnabled;
    /// \brief "pbd:sleepThreshold"
    /// 
    /// UsdPbdSolver
    const TfToken pbdSleepThreshold;
    /// \brief "pbd:startFrame"
    /// 
    /// UsdPbdSolver
    const TfToken pbdStartFrame;
    /// \brief "pbd:subSteps"
    /// 
    /// UsdPbdSolver
    const TfToken pbdSubSteps;
    /// \brief "pbd:velocity"
    /// 
    /// UsdPbdBodyAPI
    const TfToken pbdVelocity;
    /// \brief "PbdBodyAPI"
    /// 
    /// Schema identifer and family for UsdPbdBodyAPI
    const TfToken PbdBodyAPI;
    /// \brief "PbdCollisionAPI"
    /// 
    /// Schema identifer and family for UsdPbdCollisionAPI
    const TfToken PbdCollisionAPI;
    /// \brief "PbdConstraintAPI"
    /// 
    /// Schema identifer and family for UsdPbdConstraintAPI
    const TfToken PbdConstraintAPI;
    /// \brief "PbdSolver"
    /// 
    /// Schema identifer and family for UsdPbdSolver
    const TfToken PbdSolver;
    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var UsdPbdTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa UsdPbdTokensType
extern USDPBD_API TfStaticData<UsdPbdTokensType> UsdPbdTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif
