//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdNpr/contour.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdNprContour,
        TfType::Bases< UsdGeomGprim > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("Contour")
    // to find TfType<UsdNprContour>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdNprContour>("Contour");
}

/* virtual */
UsdNprContour::~UsdNprContour()
{
}

/* static */
UsdNprContour
UsdNprContour::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdNprContour();
    }
    return UsdNprContour(stage->GetPrimAtPath(path));
}

/* static */
UsdNprContour
UsdNprContour::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("Contour");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdNprContour();
    }
    return UsdNprContour(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdNprContour::_GetSchemaKind() const
{
    return UsdNprContour::schemaKind;
}

/* static */
const TfType &
UsdNprContour::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdNprContour>();
    return tfType;
}

/* static */
bool 
UsdNprContour::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdNprContour::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdNprContour::GetDrawSilhouetteAttr() const
{
    return GetPrim().GetAttribute(UsdNprTokens->drawSilhouette);
}

UsdAttribute
UsdNprContour::CreateDrawSilhouetteAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdNprTokens->drawSilhouette,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdNprContour::GetDrawBoundaryAttr() const
{
    return GetPrim().GetAttribute(UsdNprTokens->drawBoundary);
}

UsdAttribute
UsdNprContour::CreateDrawBoundaryAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdNprTokens->drawBoundary,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdNprContour::GetDrawCreaseAttr() const
{
    return GetPrim().GetAttribute(UsdNprTokens->drawCrease);
}

UsdAttribute
UsdNprContour::CreateDrawCreaseAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdNprTokens->drawCrease,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdNprContour::GetSilhouetteWidthAttr() const
{
    return GetPrim().GetAttribute(UsdNprTokens->silhouetteWidth);
}

UsdAttribute
UsdNprContour::CreateSilhouetteWidthAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdNprTokens->silhouetteWidth,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdNprContour::GetBoundaryWidthAttr() const
{
    return GetPrim().GetAttribute(UsdNprTokens->boundaryWidth);
}

UsdAttribute
UsdNprContour::CreateBoundaryWidthAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdNprTokens->boundaryWidth,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdNprContour::GetCreaseWidthAttr() const
{
    return GetPrim().GetAttribute(UsdNprTokens->creaseWidth);
}

UsdAttribute
UsdNprContour::CreateCreaseWidthAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdNprTokens->creaseWidth,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdNprContour::GetContourViewPointRel() const
{
    return GetPrim().GetRelationship(UsdNprTokens->contourViewPoint);
}

UsdRelationship
UsdNprContour::CreateContourViewPointRel() const
{
    return GetPrim().CreateRelationship(UsdNprTokens->contourViewPoint,
                       /* custom = */ false);
}

UsdRelationship
UsdNprContour::GetContourSurfacesRel() const
{
    return GetPrim().GetRelationship(UsdNprTokens->contourSurfaces);
}

UsdRelationship
UsdNprContour::CreateContourSurfacesRel() const
{
    return GetPrim().CreateRelationship(UsdNprTokens->contourSurfaces,
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
UsdNprContour::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdNprTokens->drawSilhouette,
        UsdNprTokens->drawBoundary,
        UsdNprTokens->drawCrease,
        UsdNprTokens->silhouetteWidth,
        UsdNprTokens->boundaryWidth,
        UsdNprTokens->creaseWidth,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdGeomGprim::GetSchemaAttributeNames(true),
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
PXR_NAMESPACE_OPEN_SCOPE
#include "usdNpr/tokens.h"

std::vector<UsdPrim> 
UsdNprContour::GetContourSurfaces() const
{
  
  SdfPathVector targets;
  UsdRelationship contourSurfacesRel = GetContourSurfacesRel();
  contourSurfacesRel.GetTargets(&targets);
  std::vector<UsdPrim> contourSurfaces;
  if (targets.size())
  {
    for(int i=0;i<targets.size();++i)
    {
      SdfPath primPath = targets[i].GetAbsoluteRootOrPrimPath();
      UsdPrim prim = GetPrim().GetStage()->GetPrimAtPath(primPath);
      if(TF_VERIFY(prim))
        if(prim.IsA<UsdGeomMesh>()) contourSurfaces.push_back(prim);
    }
  }
  
  if(contourSurfaces.size()) return contourSurfaces;
  
  // there was no contour surfaces authored, use the collection
  UsdCollectionAPI surfacesCollectionAPI = 
    GetContourSurfacesCollectionAPI() ;
  UsdCollectionAPI::MembershipQuery query = 
    surfacesCollectionAPI.ComputeMembershipQuery();

  SdfPathSet includedPaths = 
    UsdCollectionAPI::ComputeIncludedPaths(query, GetPrim().GetStage());
  
  for(const auto& includedPath: includedPaths)
  {
    UsdPrim prim = GetPrim().GetStage()->GetPrimAtPath(includedPath);
      if(TF_VERIFY(prim))
        if(prim.IsA<UsdGeomMesh>()) contourSurfaces.push_back(prim);
  }

  return contourSurfaces;
}

UsdCollectionAPI
UsdNprContour::GetContourSurfacesCollectionAPI() const
{
    return UsdCollectionAPI(GetPrim(), UsdNprTokens->surfaces);
}

bool
UsdNprContour::ComputeExtent(const UsdTimeCode& timeCode, const UsdPrim& prim, VtVec3fArray* extent)
{
  TfTokenVector purposes = {UsdGeomTokens->default_, UsdGeomTokens->render};
  UsdGeomBBoxCache bboxCache(timeCode, purposes, true, false);
  std::vector<UsdPrim> contourSurfaces = UsdNprContour(prim).GetContourSurfaces();
  GfRange3d accum;
  for(auto& contourSurface: contourSurfaces)
  {
    GfBBox3d bbox = bboxCache.ComputeWorldBound(contourSurface);
    accum.UnionWith(bbox.GetRange());
  }
  (*extent)[0] = GfVec3f(accum.GetMin());
  (*extent)[1] = GfVec3f(accum.GetMax());
  return true;
}

PXR_NAMESPACE_CLOSE_SCOPE
