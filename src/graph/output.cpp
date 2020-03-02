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
#include "pxr/pxr.h"
#include "output.h"

#include "connectableAPI.h"
#include "input.h"
#include "utils.h"

#include "pxr/usd/sdf/schema.h"
#include "pxr/usd/usd/relationship.h"

#include <stdlib.h>
#include <algorithm>

#include "pxr/base/tf/envSetting.h"

PXR_NAMESPACE_OPEN_SCOPE


using std::vector;
using std::string;

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (renderType)
);

GraphOutput::GraphOutput(const UsdAttribute &attr)
    : _prop(attr)
{
}

GraphOutput::GraphOutput(const UsdRelationship &rel)
    : _prop(rel)
{
}

GraphOutput::GraphOutput(const UsdProperty &prop)
    : _prop(prop)
{
}

TfToken 
GraphOutput::GetBaseName() const
{
    return TfToken(SdfPath::StripPrefixNamespace(
        GetFullName(), GraphTokens->outputs).first);
}

SdfValueTypeName 
GraphOutput::GetTypeName() const
{ 
    if (UsdAttribute attr = GetAttr()) {
        return  attr.GetTypeName();
    }

    // Fallback to token for outputs that represent terninals.
    return SdfValueTypeNames->Token;
}

static TfToken
_GetOutputAttrName(const TfToken outputName) 
{
    return TfToken(GraphTokens->outputs.GetString() + outputName.GetString());
}

GraphOutput::GraphOutput(
    UsdPrim prim,
    TfToken const &name,
    SdfValueTypeName const &typeName)
{
    // XXX what do we do if the type name doesn't match and it exists already?
    TfToken attrName = _GetOutputAttrName(name);
    _prop = prim.GetAttribute(attrName);
    if (!_prop) {
        _prop = prim.CreateAttribute(attrName, typeName, /* custom = */ false);
    }
}

bool
GraphOutput::Set(const VtValue& value,
                    UsdTimeCode time) const
{
    if (UsdAttribute attr = GetAttr()) {
        return attr.Set(value, time);
    }
    return false;
}

bool 
GraphOutput::SetRenderType(
        TfToken const& renderType) const
{
    return _prop.SetMetadata(_tokens->renderType, renderType);
}

TfToken 
GraphOutput::GetRenderType() const
{
    TfToken renderType;
    _prop.GetMetadata(_tokens->renderType, &renderType);
    return renderType;
}

bool 
GraphOutput::HasRenderType() const
{
    return _prop.HasMetadata(_tokens->renderType);
}

NdrTokenMap
GraphOutput::GetSdrMetadata() const
{
    NdrTokenMap result;

    VtDictionary sdrMetadata;
    if (GetAttr().GetMetadata(GraphTokens->sdrMetadata, &sdrMetadata)){
        for (const auto &it : sdrMetadata) {
            result[TfToken(it.first)] = TfStringify(it.second);
        }
    }

    return result;
}

std::string 
GraphOutput::GetSdrMetadataByKey(const TfToken &key) const
{
    VtValue val;
    GetAttr().GetMetadataByDictKey(GraphTokens->sdrMetadata, key, &val);
    return TfStringify(val);
}
    
void 
GraphOutput::SetSdrMetadata(const NdrTokenMap &sdrMetadata) const
{
    for (auto &i: sdrMetadata) {
        SetSdrMetadataByKey(i.first, i.second);
    }
}

void 
GraphOutput::SetSdrMetadataByKey(
    const TfToken &key, 
    const std::string &value) const
{
    GetAttr().SetMetadataByDictKey(GraphTokens->sdrMetadata, key, value);
}

bool 
GraphOutput::HasSdrMetadata() const
{
    return GetAttr().HasMetadata(GraphTokens->sdrMetadata);
}

bool 
GraphOutput::HasSdrMetadataByKey(const TfToken &key) const
{
    return GetAttr().HasMetadataDictKey(GraphTokens->sdrMetadata, key);
}

void 
GraphOutput::ClearSdrMetadata() const
{
    GetAttr().ClearMetadata(GraphTokens->sdrMetadata);
}

void
GraphOutput::ClearSdrMetadataByKey(const TfToken &key) const
{
    GetAttr().ClearMetadataByDictKey(GraphTokens->sdrMetadata, key);
}

/* static */
bool 
GraphOutput::IsOutput(const UsdAttribute &attr)
{
    return TfStringStartsWith(attr.GetName().GetString(), 
                              GraphTokens->outputs);
}

bool 
GraphOutput::CanConnect(const UsdAttribute &source) const
{
    return GraphConnectableAPI::CanConnect(*this, source);
}

bool 
GraphOutput::CanConnect(const GraphInput &sourceInput) const 
{
    return CanConnect(sourceInput.GetAttr());
}

bool 
GraphOutput::CanConnect(const GraphOutput &sourceOutput) const 
{
    return CanConnect(sourceOutput.GetAttr());
}

bool 
GraphOutput::ConnectToSource(
    GraphConnectableAPI const &source, 
    TfToken const &sourceName, 
    GraphAttributeType const sourceType,
    SdfValueTypeName typeName) const 
{
    return GraphConnectableAPI::ConnectToSource(*this, source, 
        sourceName, sourceType, typeName);   
}

bool 
GraphOutput::ConnectToSource(SdfPath const &sourcePath) const 
{
    return GraphConnectableAPI::ConnectToSource(*this, sourcePath);
}

bool 
GraphOutput::ConnectToSource(GraphInput const &sourceInput) const 
{
    return GraphConnectableAPI::ConnectToSource(*this, sourceInput);
}

bool 
GraphOutput::ConnectToSource(GraphOutput const &sourceOutput) const 
{
    return GraphConnectableAPI::ConnectToSource(*this, sourceOutput);
}

bool 
GraphOutput::GetConnectedSource(
    GraphConnectableAPI *source, 
    TfToken *sourceName,
    GraphAttributeType *sourceType) const 
{
    return GraphConnectableAPI::GetConnectedSource(*this, source, 
        sourceName, sourceType);
}

bool 
GraphOutput::GetRawConnectedSourcePaths(SdfPathVector *sourcePaths) const 
{
    return GraphConnectableAPI::GetRawConnectedSourcePaths(*this, 
        sourcePaths);
}

bool 
GraphOutput::HasConnectedSource() const 
{
    return GraphConnectableAPI::HasConnectedSource(*this);
}

bool 
GraphOutput::IsSourceConnectionFromBaseMaterial() const 
{
    return GraphConnectableAPI::IsSourceConnectionFromBaseMaterial(*this);
}

bool 
GraphOutput::DisconnectSource() const
{
    return GraphConnectableAPI::DisconnectSource(*this);
}

bool 
GraphOutput::ClearSource() const
{
    return GraphConnectableAPI::ClearSource(*this);
}

PXR_NAMESPACE_CLOSE_SCOPE

