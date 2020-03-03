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
#include "./nodeStage.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<GraphNodeStage,
        TfType::Bases< GraphNode > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("NodeStage")
    // to find TfType<GraphNodeStage>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, GraphNodeStage>("NodeStage");
}

/* virtual */
GraphNodeStage::~GraphNodeStage()
{
}

/* static */
GraphNodeStage
GraphNodeStage::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GraphNodeStage();
    }
    return GraphNodeStage(stage->GetPrimAtPath(path));
}

/* static */
GraphNodeStage
GraphNodeStage::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("NodeStage");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GraphNodeStage();
    }
    return GraphNodeStage(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaType GraphNodeStage::_GetSchemaType() const {
    return GraphNodeStage::schemaType;
}

/* static */
const TfType &
GraphNodeStage::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<GraphNodeStage>();
    return tfType;
}

/* static */
bool 
GraphNodeStage::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
GraphNodeStage::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
GraphNodeStage::GetLifetimeManagementAttr() const
{
    return GetPrim().GetAttribute(GraphTokens->lifetimeManagement);
}

UsdAttribute
GraphNodeStage::CreateLifetimeManagementAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(GraphTokens->lifetimeManagement,
                       SdfValueTypeNames->Token,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraphNodeStage::GetFileNameAttr() const
{
    return GetPrim().GetAttribute(GraphTokens->fileName);
}

UsdAttribute
GraphNodeStage::CreateFileNameAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(GraphTokens->fileName,
                       SdfValueTypeNames->String,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraphNodeStage::GetLoadPrimsPathAttr() const
{
    return GetPrim().GetAttribute(GraphTokens->loadPrimsPath);
}

UsdAttribute
GraphNodeStage::CreateLoadPrimsPathAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(GraphTokens->loadPrimsPath,
                       SdfValueTypeNames->StringArray,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraphNodeStage::GetLoadPrimsStatesAttr() const
{
    return GetPrim().GetAttribute(GraphTokens->loadPrimsStates);
}

UsdAttribute
GraphNodeStage::CreateLoadPrimsStatesAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(GraphTokens->loadPrimsStates,
                       SdfValueTypeNames->TokenArray,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraphNodeStage::GetPopulationMaskAttr() const
{
    return GetPrim().GetAttribute(GraphTokens->populationMask);
}

UsdAttribute
GraphNodeStage::CreatePopulationMaskAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(GraphTokens->populationMask,
                       SdfValueTypeNames->StringArray,
                       /* custom = */ false,
                       SdfVariabilityUniform,
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
GraphNodeStage::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        GraphTokens->lifetimeManagement,
        GraphTokens->fileName,
        GraphTokens->loadPrimsPath,
        GraphTokens->loadPrimsStates,
        GraphTokens->populationMask,
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
