//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdPbd/bodyAPI.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPbdBodyAPI,
        TfType::Bases< UsdAPISchemaBase > >();
    
}

/* virtual */
UsdPbdBodyAPI::~UsdPbdBodyAPI()
{
}

/* static */
UsdPbdBodyAPI
UsdPbdBodyAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPbdBodyAPI();
    }
    return UsdPbdBodyAPI(stage->GetPrimAtPath(path));
}


/* virtual */
UsdSchemaKind UsdPbdBodyAPI::_GetSchemaKind() const
{
    return UsdPbdBodyAPI::schemaKind;
}

/* static */
bool
UsdPbdBodyAPI::CanApply(
    const UsdPrim &prim, std::string *whyNot)
{
    return prim.CanApplyAPI<UsdPbdBodyAPI>(whyNot);
}

/* static */
UsdPbdBodyAPI
UsdPbdBodyAPI::Apply(const UsdPrim &prim)
{
    if (prim.ApplyAPI<UsdPbdBodyAPI>()) {
        return UsdPbdBodyAPI(prim);
    }
    return UsdPbdBodyAPI();
}

/* static */
const TfType &
UsdPbdBodyAPI::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPbdBodyAPI>();
    return tfType;
}

/* static */
bool 
UsdPbdBodyAPI::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPbdBodyAPI::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPbdBodyAPI::GetSimulationEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdSimulationEnabled);
}

UsdAttribute
UsdPbdBodyAPI::CreateSimulationEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdSimulationEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetRadiusAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdRadius);
}

UsdAttribute
UsdPbdBodyAPI::CreateRadiusAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdRadius,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetSelfCollisionEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdSelfCollisionEnabled);
}

UsdAttribute
UsdPbdBodyAPI::CreateSelfCollisionEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdSelfCollisionEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetSelfCollisionRadiusAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdSelfCollisionRadius);
}

UsdAttribute
UsdPbdBodyAPI::CreateSelfCollisionRadiusAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdSelfCollisionRadius,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetSelfCollisionDampAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdSelfCollisionDamp);
}

UsdAttribute
UsdPbdBodyAPI::CreateSelfCollisionDampAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdSelfCollisionDamp,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetSelfCollisionFrictionAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdSelfCollisionFriction);
}

UsdAttribute
UsdPbdBodyAPI::CreateSelfCollisionFrictionAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdSelfCollisionFriction,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetSelfCollisionRestitutionAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdSelfCollisionRestitution);
}

UsdAttribute
UsdPbdBodyAPI::CreateSelfCollisionRestitutionAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdSelfCollisionRestitution,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetSelfCollisionMaxSeparationVelocityAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdSelfCollisionMaxSeparationVelocity);
}

UsdAttribute
UsdPbdBodyAPI::CreateSelfCollisionMaxSeparationVelocityAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdSelfCollisionMaxSeparationVelocity,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetMassAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdMass);
}

UsdAttribute
UsdPbdBodyAPI::CreateMassAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdMass,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetDampAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdDamp);
}

UsdAttribute
UsdPbdBodyAPI::CreateDampAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdDamp,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdBodyAPI::GetVelocityAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdVelocity);
}

UsdAttribute
UsdPbdBodyAPI::CreateVelocityAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdVelocity,
                       SdfValueTypeNames->Vector3f,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
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
UsdPbdBodyAPI::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPbdTokens->pbdSimulationEnabled,
        UsdPbdTokens->pbdRadius,
        UsdPbdTokens->pbdSelfCollisionEnabled,
        UsdPbdTokens->pbdSelfCollisionRadius,
        UsdPbdTokens->pbdSelfCollisionDamp,
        UsdPbdTokens->pbdSelfCollisionFriction,
        UsdPbdTokens->pbdSelfCollisionRestitution,
        UsdPbdTokens->pbdSelfCollisionMaxSeparationVelocity,
        UsdPbdTokens->pbdMass,
        UsdPbdTokens->pbdDamp,
        UsdPbdTokens->pbdVelocity,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdAPISchemaBase::GetSchemaAttributeNames(true),
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
