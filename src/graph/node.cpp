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

UsdAttribute
GraphNode::GetImplementationSourceAttr() const
{
    return GetPrim().GetAttribute(GraphTokens->infoImplementationSource);
}

UsdAttribute
GraphNode::CreateImplementationSourceAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(GraphTokens->infoImplementationSource,
                       SdfValueTypeNames->Token,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
GraphNode::GetIdAttr() const
{
    return GetPrim().GetAttribute(GraphTokens->infoId);
}

UsdAttribute
GraphNode::CreateIdAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(GraphTokens->infoId,
                       SdfValueTypeNames->Token,
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
GraphNode::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        GraphTokens->infoImplementationSource,
        GraphTokens->infoId,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdTyped::GetSchemaAttributeNames(true),
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

TfToken 
GraphNode::GetImplementationSource() const
{
    TfToken implSource;
    GetImplementationSourceAttr().Get(&implSource);

    if (implSource == GraphTokens->id ||
        implSource == GraphTokens->sourceAsset ||
        implSource == GraphTokens->sourceCode) {
        return implSource;
    } else {
        TF_WARN("Found invalid info:implementationSource value '%s' on shader "
                "at path <%s>. Falling back to 'id'.", implSource.GetText(),
                GetPath().GetText());
        return GraphTokens->id;
    }
}
    
bool 
GraphNode::SetShaderId(const TfToken &id) const
{
    return CreateImplementationSourceAttr(VtValue(GraphTokens->id), 
                                          /*writeSparsely*/ true) &&
           GetIdAttr().Set(id);
}

bool 
GraphNode::GetShaderId(TfToken *id) const
{
    TfToken implSource = GetImplementationSource();
    if (implSource == GraphTokens->id) {
        return GetIdAttr().Get(id);
    }
    return false;
}

static 
TfToken
_GetSourceAssetAttrName(const TfToken &sourceType) 
{
    if (sourceType == GraphTokens->universalSourceType) {
        return _tokens->infoSourceAsset;
    }
    return TfToken(SdfPath::JoinIdentifier(TfTokenVector{
                                    _tokens->info, 
                                    sourceType,
                                    GraphTokens->sourceAsset}));
}

bool 
GraphNode::SetSourceAsset(
    const SdfAssetPath &sourceAsset,
    const TfToken &sourceType) const
{
    TfToken sourceAssetAttrName = _GetSourceAssetAttrName(sourceType);
    return CreateImplementationSourceAttr(VtValue(GraphTokens->sourceAsset)) 
        && UsdSchemaBase::_CreateAttr(sourceAssetAttrName,
                                      SdfValueTypeNames->Asset,
                                      /* custom = */ false,
                                      SdfVariabilityUniform,
                                      VtValue(sourceAsset),
                                      /* writeSparsely */ false);
}

bool 
GraphNode::GetSourceAsset(
    SdfAssetPath *sourceAsset,
    const TfToken &sourceType) const
{
    TfToken implSource = GetImplementationSource();
    if (implSource != GraphTokens->sourceAsset) {
        return false;
    }

    TfToken sourceAssetAttrName = _GetSourceAssetAttrName(sourceType);
    UsdAttribute sourceAssetAttr = GetPrim().GetAttribute(sourceAssetAttrName);
    if (sourceAssetAttr) {
        return sourceAssetAttr.Get(sourceAsset);
    }

    if (sourceType != GraphTokens->universalSourceType) {
        UsdAttribute univSourceAssetAttr = GetPrim().GetAttribute(
                _GetSourceAssetAttrName(GraphTokens->universalSourceType));
        if (univSourceAssetAttr) {
            return univSourceAssetAttr.Get(sourceAsset);
        }
    }

    return false;
}

static 
TfToken
_GetSourceCodeAttrName(const TfToken &sourceType) 
{
    if (sourceType == GraphTokens->universalSourceType) {
        return _tokens->infoSourceCode;
    }
    return TfToken(SdfPath::JoinIdentifier(TfTokenVector{
                                    _tokens->info, 
                                    sourceType,
                                    GraphTokens->sourceCode}));
}

bool 
GraphNode::SetSourceCode(
    const std::string &sourceCode, 
    const TfToken &sourceType) const
{
    TfToken sourceCodeAttrName = _GetSourceCodeAttrName(sourceType);
    return CreateImplementationSourceAttr(VtValue(GraphTokens->sourceCode)) 
        && UsdSchemaBase::_CreateAttr(sourceCodeAttrName,
                                      SdfValueTypeNames->String,
                                      /* custom = */ false,
                                      SdfVariabilityUniform,
                                      VtValue(sourceCode),
                                      /* writeSparsely */ false);
}

bool 
GraphNode::GetSourceCode(
    std::string *sourceCode,
    const TfToken &sourceType) const
{
    TfToken implSource = GetImplementationSource();
    if (implSource != GraphTokens->sourceCode) {
        return false;
    }

    TfToken sourceCodeAttrName = _GetSourceCodeAttrName(sourceType);
    UsdAttribute sourceCodeAttr = GetPrim().GetAttribute(sourceCodeAttrName);
    if (sourceCodeAttr) {
        return sourceCodeAttr.Get(sourceCode);
    }

    if (sourceType != GraphTokens->universalSourceType) {
        UsdAttribute univSourceCodeAttr = GetPrim().GetAttribute(
                _GetSourceCodeAttrName(GraphTokens->universalSourceType));
        if (univSourceCodeAttr) {
            return univSourceCodeAttr.Get(sourceCode);
        }
    }

    return false;
}

/*
SdrShaderNodeConstPtr 
GraphNode::GetShaderNodeForSourceType(const TfToken &sourceType) const
{
    TfToken implSource = GetImplementationSource();
    if (implSource == GraphTokens->id) {
        TfToken shaderId;
        if (GetShaderId(&shaderId)) {
            return SdrRegistry::GetInstance().GetShaderNodeByIdentifierAndType(
                    shaderId, sourceType);
        }
    } else if (implSource == GraphTokens->sourceAsset) {
        SdfAssetPath sourceAsset;
        if (GetSourceAsset(&sourceAsset, sourceType)) {
            return SdrRegistry::GetInstance().GetShaderNodeFromAsset(
                sourceAsset, GetSdrMetadata());
        }
    } else if (implSource == GraphTokens->sourceCode) {
        std::string sourceCode;
        if (GetSourceCode(&sourceCode, sourceType)) {
            return SdrRegistry::GetInstance().GetShaderNodeFromSourceCode(
                sourceCode, sourceType, GetSdrMetadata());
        }
    }

    return nullptr;
}*/

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
