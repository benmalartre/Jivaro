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
#include "./node.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<GraphNode,
        TfType::Bases< UsdTyped > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("Node")
    // to find TfType<GraphNode>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, GraphNode>("Node");
}

/* virtual */
GraphNode::~GraphNode()
{
}

/* static */
GraphNode
GraphNode::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GraphNode();
    }
    return GraphNode(stage->GetPrimAtPath(path));
}

/* static */
GraphNode
GraphNode::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("Node");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GraphNode();
    }
    return GraphNode(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaType GraphNode::_GetSchemaType() const {
    return GraphNode::schemaType;
}

/* static */
const TfType &
GraphNode::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<GraphNode>();
    return tfType;
}

/* static */
bool 
GraphNode::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
GraphNode::_GetTfType() const
{
    return _GetStaticTfType();
}

/*static*/
const TfTokenVector&
GraphNode::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames;
    static TfTokenVector allNames =
        UsdTyped::GetSchemaAttributeNames(true);

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

#include "pxr/usd/sdr/registry.h"
#include "connectableAPI.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (info)
    ((infoSourceAsset, "info:sourceAsset"))
    ((infoSourceCode, "info:sourceCode"))
);

GraphNode::GraphNode(const GraphConnectableAPI &connectable)
    : GraphNode(connectable.GetPrim())
{
}

GraphConnectableAPI 
GraphNode::ConnectableAPI() const
{
    return GraphConnectableAPI(GetPrim());
}

GraphOutput
GraphNode::CreateOutput(const TfToken& name,
                             const SdfValueTypeName& typeName)
{
    return GraphConnectableAPI(GetPrim()).CreateOutput(name, typeName);
}

GraphOutput
GraphNode::GetOutput(const TfToken &name) const
{
    return GraphConnectableAPI(GetPrim()).GetOutput(name);
}

std::vector<GraphOutput>
GraphNode::GetOutputs() const
{
    return GraphConnectableAPI(GetPrim()).GetOutputs();
}

GraphInput
GraphNode::CreateInput(const TfToken& name,
                            const SdfValueTypeName& typeName)
{
    return GraphConnectableAPI(GetPrim()).CreateInput(name, typeName);
}

GraphInput
GraphNode::GetInput(const TfToken &name) const
{
    return GraphConnectableAPI(GetPrim()).GetInput(name);
}

std::vector<GraphInput>
GraphNode::GetInputs() const
{
    return GraphConnectableAPI(GetPrim()).GetInputs();
}

NdrTokenMap
GraphNode::GetSdrMetadata() const
{
    NdrTokenMap result;

    VtDictionary sdrMetadata;
    if (GetPrim().GetMetadata(GraphTokens->sdrMetadata, &sdrMetadata)){
        for (const auto &it : sdrMetadata) {
            result[TfToken(it.first)] = TfStringify(it.second);
        }
    }

    return result;
}

std::string 
GraphNode::GetSdrMetadataByKey(const TfToken &key) const
{
    VtValue val;
    GetPrim().GetMetadataByDictKey(GraphTokens->sdrMetadata, key, &val);
    return TfStringify(val);
}
    
void 
GraphNode::SetSdrMetadata(const NdrTokenMap &sdrMetadata) const
{
    for (auto &i: sdrMetadata) {
        SetSdrMetadataByKey(i.first, i.second);
    }
}

void 
GraphNode::SetSdrMetadataByKey(
    const TfToken &key, 
    const std::string &value) const
{
    GetPrim().SetMetadataByDictKey(GraphTokens->sdrMetadata, key, value);
}

bool 
GraphNode::HasSdrMetadata() const
{
    return GetPrim().HasMetadata(GraphTokens->sdrMetadata);
}

bool 
GraphNode::HasSdrMetadataByKey(const TfToken &key) const
{
    return GetPrim().HasMetadataDictKey(GraphTokens->sdrMetadata, key);
}

void 
GraphNode::ClearSdrMetadata() const
{
    GetPrim().ClearMetadata(GraphTokens->sdrMetadata);
}

void
GraphNode::ClearSdrMetadataByKey(const TfToken &key) const
{
    GetPrim().ClearMetadataByDictKey(GraphTokens->sdrMetadata, key);
}

PXR_NAMESPACE_CLOSE_SCOPE
