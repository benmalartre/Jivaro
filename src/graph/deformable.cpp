//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "./deformable.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<GraphDeformable,
        TfType::Bases< GraphNode > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("Deformable")
    // to find TfType<GraphDeformable>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, GraphDeformable>("Deformable");
}

/* virtual */
GraphDeformable::~GraphDeformable()
{
}

/* static */
GraphDeformable
GraphDeformable::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GraphDeformable();
    }
    return GraphDeformable(stage->GetPrimAtPath(path));
}

/* static */
GraphDeformable
GraphDeformable::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("Deformable");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GraphDeformable();
    }
    return GraphDeformable(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaType GraphDeformable::_GetSchemaType() const {
    return GraphDeformable::schemaType;
}

/* static */
const TfType &
GraphDeformable::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<GraphDeformable>();
    return tfType;
}

/* static */
bool 
GraphDeformable::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
GraphDeformable::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
GraphDeformable::GetGeometryAttr() const
{
    return GetPrim().GetAttribute(GraphTokens->inputsGeometry);
}

UsdAttribute
GraphDeformable::CreateGeometryAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(GraphTokens->inputsGeometry,
                       SdfValueTypeNames->Token,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraphDeformable::GetDeformedAttr() const
{
    return GetPrim().GetAttribute(GraphTokens->outputsDeformed);
}

UsdAttribute
GraphDeformable::CreateDeformedAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(GraphTokens->outputsDeformed,
                       SdfValueTypeNames->Token,
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
GraphDeformable::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        GraphTokens->inputsGeometry,
        GraphTokens->outputsDeformed,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            GraphNode::GetSchemaAttributeNames(true),
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
