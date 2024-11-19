//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdPbd/collisionAPI.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPbdCollisionAPI,
        TfType::Bases< UsdAPISchemaBase > >();
    
}

/* virtual */
UsdPbdCollisionAPI::~UsdPbdCollisionAPI()
{
}

/* static */
UsdPbdCollisionAPI
UsdPbdCollisionAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPbdCollisionAPI();
    }
    return UsdPbdCollisionAPI(stage->GetPrimAtPath(path));
}


/* virtual */
UsdSchemaKind UsdPbdCollisionAPI::_GetSchemaKind() const
{
    return UsdPbdCollisionAPI::schemaKind;
}

/* static */
bool
UsdPbdCollisionAPI::CanApply(
    const UsdPrim &prim, std::string *whyNot)
{
    return prim.CanApplyAPI<UsdPbdCollisionAPI>(whyNot);
}

/* static */
UsdPbdCollisionAPI
UsdPbdCollisionAPI::Apply(const UsdPrim &prim)
{
    if (prim.ApplyAPI<UsdPbdCollisionAPI>()) {
        return UsdPbdCollisionAPI(prim);
    }
    return UsdPbdCollisionAPI();
}

/* static */
const TfType &
UsdPbdCollisionAPI::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPbdCollisionAPI>();
    return tfType;
}

/* static */
bool 
UsdPbdCollisionAPI::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPbdCollisionAPI::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPbdCollisionAPI::GetCollisionEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdCollisionEnabled);
}

UsdAttribute
UsdPbdCollisionAPI::CreateCollisionEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdCollisionEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdCollisionAPI::GetMarginAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdMargin);
}

UsdAttribute
UsdPbdCollisionAPI::CreateMarginAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdMargin,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdCollisionAPI::GetDampAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdCollisionDamp);
}

UsdAttribute
UsdPbdCollisionAPI::CreateDampAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdCollisionDamp,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdCollisionAPI::GetCollisionStiffnessAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdCollisionStiffness);
}

UsdAttribute
UsdPbdCollisionAPI::CreateCollisionStiffnessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdCollisionStiffness,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdCollisionAPI::GetFrictionAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdFriction);
}

UsdAttribute
UsdPbdCollisionAPI::CreateFrictionAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdFriction,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdCollisionAPI::GetRestitutionAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdRestitution);
}

UsdAttribute
UsdPbdCollisionAPI::CreateRestitutionAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdRestitution,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPbdCollisionAPI::GetMaxSeparationVelocityAttr() const
{
    return GetPrim().GetAttribute(UsdPbdTokens->pbdMaxSeparationVelocity);
}

UsdAttribute
UsdPbdCollisionAPI::CreateMaxSeparationVelocityAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPbdTokens->pbdMaxSeparationVelocity,
                       SdfValueTypeNames->Float,
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
UsdPbdCollisionAPI::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPbdTokens->pbdCollisionEnabled,
        UsdPbdTokens->pbdMargin,
        UsdPbdTokens->pbdCollisionDamp,
        UsdPbdTokens->pbdCollisionStiffness,
        UsdPbdTokens->pbdFriction,
        UsdPbdTokens->pbdRestitution,
        UsdPbdTokens->pbdMaxSeparationVelocity,
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
