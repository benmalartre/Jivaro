//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdPbd/solver.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPbdSolver,
        TfType::Bases< UsdGeomXform > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("PbdSolver")
    // to find TfType<UsdPbdSolver>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdPbdSolver>("PbdSolver");
}

/* virtual */
UsdPbdSolver::~UsdPbdSolver()
{
}

/* static */
UsdPbdSolver
UsdPbdSolver::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPbdSolver();
    }
    return UsdPbdSolver(stage->GetPrimAtPath(path));
}

/* static */
UsdPbdSolver
UsdPbdSolver::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("PbdSolver");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPbdSolver();
    }
    return UsdPbdSolver(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdPbdSolver::_GetSchemaKind() const
{
    return UsdPbdSolver::schemaKind;
}

/* static */
const TfType &
UsdPbdSolver::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPbdSolver>();
    return tfType;
}

/* static */
bool 
UsdPbdSolver::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPbdSolver::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPbdSolver::GetStartFrameAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdStartFrame);
}

UsdAttribute
UsdPbdSolver::CreateStartFrameAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdStartFrame,
                       SdfValueTypeNames->Int,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdSolver::GetSleepThresholdAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdSleepThreshold);
}

UsdAttribute
UsdPbdSolver::CreateSleepThresholdAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdSleepThreshold,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdSolver::GetSubStepsAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdSubSteps);
}

UsdAttribute
UsdPbdSolver::CreateSubStepsAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdSubSteps,
                       SdfValueTypeNames->Int,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdSolver::GetGravityAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdGravity);
}

UsdAttribute
UsdPbdSolver::CreateGravityAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdGravity,
                       SdfValueTypeNames->Vector3f,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdSolver::GetShowPointsAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdShowPoints);
}

UsdAttribute
UsdPbdSolver::CreateShowPointsAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdShowPoints,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdSolver::GetShowConstraintsAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdShowConstraints);
}

UsdAttribute
UsdPbdSolver::CreateShowConstraintsAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdShowConstraints,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdPbdSolver::GetBodiesRel() const
{
    return GetPrim().GetRelationship(UsdPbdTokens->pbdBodies);
}

UsdRelationship
UsdPbdSolver::CreateBodiesRel() const
{
    return GetPrim().CreateRelationship(UsdPbdTokens->pbdBodies,
                       /* custom = */ false);
}

UsdRelationship
UsdPbdSolver::GetCollidersRel() const
{
    return GetPrim().GetRelationship(UsdPbdTokens->pbdColliders);
}

UsdRelationship
UsdPbdSolver::CreateCollidersRel() const
{
    return GetPrim().CreateRelationship(UsdPbdTokens->pbdColliders,
                       /* custom = */ false);
}

namespace {
static inline TfTokenVector
_ConcatenateAttributeNames(const TfTokenVector& left,const TfTokenVector& right)
{
    TfTokenVector result;
    result.reserve(left.size() + right.size());
    result.insert(result.end(), left.begin(), left.end());
    result.insert(result.end(), right.begin(), right.end());
    return result;
}
}

/*static*/
const TfTokenVector&
UsdPbdSolver::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPbdTokens->pbdStartFrame,
        UsdPbdTokens->pbdSleepThreshold,
        UsdPbdTokens->pbdSubSteps,
        UsdPbdTokens->pbdGravity,
        UsdPbdTokens->pbdShowPoints,
        UsdPbdTokens->pbdShowConstraints,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdGeomXform::GetSchemaAttributeNames(true),
            localNames);

    if (includeInherited)
        return allNames;
    else
        return localNames;
}

PXR_NAMESPACE_CLOSE_SCOPE

// ===================================================================== //
// Feel free to add custom code below this line. It will be preserved by
// the code generator.
//
// Just remember to wrap code in the appropriate delimiters:
// 'PXR_NAMESPACE_OPEN_SCOPE', 'PXR_NAMESPACE_CLOSE_SCOPE'.
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--
