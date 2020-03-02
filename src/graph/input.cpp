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
#include "pxr/usd/sdf/schema.h"
#include "pxr/usd/usd/relationship.h"
#include "pxr/base/tf/smallVector.h"
#include "pxr/base/tf/envSetting.h"

#include "input.h"
#include "connectableAPI.h"
#include "output.h"
#include "utils.h"

#include <stdlib.h>
#include <algorithm>

PXR_NAMESPACE_OPEN_SCOPE


using std::vector;
using std::string;

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (connectability)
    (renderType)
);

GraphInput::GraphInput(const UsdAttribute &attr)
    : _attr(attr)
{
}

TfToken 
GraphInput::GetBaseName() const
{
    string name = GetFullName();
    if (TfStringStartsWith(name, GraphTokens->inputs)) {
        return TfToken(name.substr(GraphTokens->inputs.GetString().size()));
    } else if (GraphUtils::ReadOldEncoding() && 
               TfStringStartsWith(name, GraphTokens->interface_)) {
        return TfToken(name.substr(
            GraphTokens->interface_.GetString().size()));
    }
    
    return GetFullName();
}

SdfValueTypeName 
GraphInput::GetTypeName() const
{ 
    return _attr.GetTypeName();
}

static TfToken
_GetInputAttrName(const TfToken inputName) 
{
    return TfToken(GraphTokens->inputs.GetString() + inputName.GetString());
}

GraphInput::GraphInput(
    UsdPrim prim,
    TfToken const &name,
    SdfValueTypeName const &typeName)
{
    // XXX what do we do if the type name doesn't match and it exists already?
    TfToken inputAttrName = _GetInputAttrName(name);
    if (prim.HasAttribute(inputAttrName)) {
        _attr = prim.GetAttribute(inputAttrName);
    }

    if (GraphUtils::ReadOldEncoding()) {
        if (prim.HasAttribute(name)) {
            _attr = prim.GetAttribute(name);
        }
        else {
            TfToken interfaceAttrName(GraphTokens->interface_.GetString() + 
                                      name.GetString());
            if (prim.HasAttribute(interfaceAttrName)) {
                _attr = prim.GetAttribute(interfaceAttrName);
            }
        }
    }

    if (!_attr) {
        if (GraphUtils::WriteNewEncoding()) {
            _attr = prim.CreateAttribute(inputAttrName, typeName, 
                /* custom = */ false);
        } else {
            GraphConnectableAPI connectable(prim);
            // If this is a node-graph and the name already contains "interface:" 
            // namespace in it, just create the attribute with the requested 
            // name.
            if (connectable.IsNodeGraph()) { 
                TfToken attrName = TfStringStartsWith(name.GetString(),
                    GraphTokens->interface_) ? name : 
                    TfToken(GraphTokens->interface_.GetString() + name.GetString());
                _attr = prim.CreateAttribute(attrName, typeName, /*custom*/ false);
            } else {
                // fallback to creating an old style GraphParameter.
                _attr = prim.CreateAttribute(name, typeName, /*custom*/ false);
            }
        }
    }
}

bool
GraphInput::Get(VtValue* value, UsdTimeCode time) const
{
    if (!_attr) {
        return false;
    }

    return _attr.Get(value, time);
}

bool
GraphInput::Set(const VtValue& value, UsdTimeCode time) const
{
    return _attr.Set(value, time);
}

bool 
GraphInput::SetRenderType(TfToken const& renderType) const
{
    return _attr.SetMetadata(_tokens->renderType, renderType);
}

TfToken 
GraphInput::GetRenderType() const
{
    TfToken renderType;
    _attr.GetMetadata(_tokens->renderType, &renderType);
    return renderType;
}

bool 
GraphInput::HasRenderType() const
{
    return _attr.HasMetadata(_tokens->renderType);
}


NdrTokenMap
GraphInput::GetSdrMetadata() const
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
GraphInput::GetSdrMetadataByKey(const TfToken &key) const
{
    VtValue val;
    GetAttr().GetMetadataByDictKey(GraphTokens->sdrMetadata, key, &val);
    return TfStringify(val);
}
    
void 
GraphInput::SetSdrMetadata(const NdrTokenMap &sdrMetadata) const
{
    for (auto &i: sdrMetadata) {
        SetSdrMetadataByKey(i.first, i.second);
    }
}

void 
GraphInput::SetSdrMetadataByKey(
    const TfToken &key, 
    const std::string &value) const
{
    GetAttr().SetMetadataByDictKey(GraphTokens->sdrMetadata, key, value);
}

bool 
GraphInput::HasSdrMetadata() const
{
    return GetAttr().HasMetadata(GraphTokens->sdrMetadata);
}

bool 
GraphInput::HasSdrMetadataByKey(const TfToken &key) const
{
    return GetAttr().HasMetadataDictKey(GraphTokens->sdrMetadata, key);
}

void 
GraphInput::ClearSdrMetadata() const
{
    GetAttr().ClearMetadata(GraphTokens->sdrMetadata);
}

void
GraphInput::ClearSdrMetadataByKey(const TfToken &key) const
{
    GetAttr().ClearMetadataByDictKey(GraphTokens->sdrMetadata, key);
}

/* static */
bool 
GraphInput::IsInput(const UsdAttribute &attr)
{
    return attr && attr.IsDefined() && 
            // If reading of old encoding is supported, then assume it's      
            // an input as long as it's not in the "outputs:" namespace.
            // If support for reading the old encoding is disabled, then only
            // identify as an input if the attr is in the "inputs:" namespace.
            (GraphUtils::ReadOldEncoding() ? 
             !TfStringStartsWith(attr.GetName().GetString(), 
                                GraphTokens->outputs) :
             TfStringStartsWith(attr.GetName().GetString(), 
                                GraphTokens->inputs));
}

/* static */
bool
GraphInput::IsInterfaceInputName(const std::string & name)
{
    if (TfStringStartsWith(name, GraphTokens->inputs)) {
        return true;
    }

    if (GraphUtils::ReadOldEncoding() &&
        TfStringStartsWith(name, GraphTokens->interface_)) {
        return true;
    }

    return false;
}

bool 
GraphInput::SetDocumentation(const std::string& docs) const
{
    if (!_attr) {
        return false;
    }

    return _attr.SetDocumentation(docs);
}

std::string 
GraphInput::GetDocumentation() const
{
    if (!_attr) {
        return "";
    }

    return _attr.GetDocumentation();
}

bool 
GraphInput::SetDisplayGroup(const std::string& docs) const
{
    if (!_attr) {
        return false;
    }

    return _attr.SetDisplayGroup(docs);
}

std::string 
GraphInput::GetDisplayGroup() const
{
    if (!_attr) {
        return "";
    }

    return _attr.GetDisplayGroup();
}

bool 
GraphInput::CanConnect(const UsdAttribute &source) const 
{
    return GraphConnectableAPI::CanConnect(*this, source);
}

bool 
GraphInput::CanConnect(const GraphInput &sourceInput) const 
{
    return CanConnect(sourceInput.GetAttr());
}

bool 
GraphInput::CanConnect(const GraphOutput &sourceOutput) const
{
    return CanConnect(sourceOutput.GetAttr());
}

bool 
GraphInput::ConnectToSource(
    GraphConnectableAPI const &source, 
    TfToken const &sourceName, 
    GraphAttributeType const sourceType,
    SdfValueTypeName typeName) const 
{
    return GraphConnectableAPI::ConnectToSource(*this, source, 
        sourceName, sourceType, typeName);   
}

bool 
GraphInput::ConnectToSource(SdfPath const &sourcePath) const 
{
    return GraphConnectableAPI::ConnectToSource(*this, sourcePath);
}

bool 
GraphInput::ConnectToSource(GraphInput const &sourceInput) const 
{
    return GraphConnectableAPI::ConnectToSource(*this, sourceInput);
}

bool 
GraphInput::ConnectToSource(GraphOutput const &sourceOutput) const 
{
    return GraphConnectableAPI::ConnectToSource(*this, sourceOutput);
}

bool 
GraphInput::GetConnectedSource(GraphConnectableAPI *source, 
                                  TfToken *sourceName,
                                  GraphAttributeType *sourceType) const 
{
    return GraphConnectableAPI::GetConnectedSource(*this, source, 
        sourceName, sourceType);
}

bool 
GraphInput::GetRawConnectedSourcePaths(SdfPathVector *sourcePaths) const 
{
    return GraphConnectableAPI::GetRawConnectedSourcePaths(*this, 
        sourcePaths);
}

bool 
GraphInput::HasConnectedSource() const 
{
    return GraphConnectableAPI::HasConnectedSource(*this);
}

bool 
GraphInput::IsSourceConnectionFromBaseMaterial() const 
{
    return GraphConnectableAPI::IsSourceConnectionFromBaseMaterial(*this);
}

bool 
GraphInput::DisconnectSource() const 
{
    return GraphConnectableAPI::DisconnectSource(*this);
}

bool 
GraphInput::ClearSource() const 
{
    return GraphConnectableAPI::ClearSource(*this);
}

bool 
GraphInput::SetConnectability(const TfToken &connectability) const
{
    return _attr.SetMetadata(_tokens->connectability, connectability);
}

TfToken 
GraphInput::GetConnectability() const
{
    TfToken connectability; 
    _attr.GetMetadata(_tokens->connectability, &connectability);

    // If there's an authored non-empty connectability value, then return it. 
    // If not, return "full".
    if (!connectability.IsEmpty()) {
        return connectability;
    }

    return GraphTokens->full;
}

bool 
GraphInput::ClearConnectability() const
{
    return _attr.ClearMetadata(_tokens->connectability);
}

// Note: to avoid getting stuck in an infinite loop when following connections,
// we need to check if we've visited an attribute before, so that we can break
// the cycle and return an invalid result.
// We expect most connections chains to be very small with most of them having
// 0 or 1 connection in the chain. Few will include multiple hops. That is why
// we are going with a vector and not a set to check for previous attributes.
// To avoid the cost of allocating memory on the heap at each invocation, we
// use a TfSmallVector to keep the first couple of entries on the stack.
constexpr unsigned int N = 5;
typedef TfSmallVector<SdfPath, N> _SmallSdfPathVector;

template <typename GraphInOutput>
std::pair<UsdAttribute, GraphAttributeType>
_GetValueProducingAttributeRecursive(GraphInOutput const & inoutput,
                                     _SmallSdfPathVector& foundAttributes)
{
    UsdAttribute attr;
    GraphAttributeType attrType = GraphAttributeType::Invalid;
    if (!inoutput) {
        return std::make_pair(attr, attrType);
    }

    constexpr bool isInput =
            std::is_same<GraphInOutput, GraphInput>::value;

    // Check if we've visited this attribute before and if so abort with an
    // error, since this means we have a loop in the chain
    const SdfPath& thisAttrPath = inoutput.GetAttr().GetPath();
    if (std::find(foundAttributes.begin(), foundAttributes.end(),
                  thisAttrPath) != foundAttributes.end()) {
        TF_WARN("GetValueProducingAttribute: Found cycle with attribute %s",
                thisAttrPath.GetText());
        return std::make_pair(attr, attrType);
    }

    // Remember the path of this attribute, so that we do not visit it again
    foundAttributes.push_back(thisAttrPath);

    // Check if this input or output is connected to anything
    GraphConnectableAPI source;
    TfToken sourceName;
    GraphAttributeType sourceType;
    if (GraphConnectableAPI::GetConnectedSource(inoutput,
                &source, &sourceName, &sourceType)) {

        // If it is connected follow it until we reach an attribute on an
        // actual shader node
        if (sourceType == GraphAttributeType::Output) {
            GraphOutput connectedOutput = source.GetOutput(sourceName);
            if (source.IsNode()) {
                attr = connectedOutput.GetAttr();
                attrType = GraphAttributeType::Output;
            } else {
                std::tie(attr, attrType) =
                        _GetValueProducingAttributeRecursive(connectedOutput,
                                                             foundAttributes);
            }
        } else if (sourceType == GraphAttributeType::Input) {
            GraphInput connectedInput = source.GetInput(sourceName);
            if (source.IsNode()) {
                // Note, this is an invalid situation for a connected chain.
                // Since we started on an input to either a Shader or a
                // NodeGraph we cannot legally connect to an input on a Shader.
            } else {
                std::tie(attr, attrType) =
                        _GetValueProducingAttributeRecursive(connectedInput,
                                                             foundAttributes);
            }
        }

    } else {
        // The attribute is not connected. If there is a value it is coming
        // from either this attribute or a previously encountered one
        attrType = isInput ? GraphAttributeType::Input :
                             GraphAttributeType::Output;
    }

    // If we haven't found a valid value yet and the current input has an
    // authored value, then return this attribute.
    // Note, we can get here after encountering an error in a deeper recursion
    // which would return an invalid attribute with an invalid type. If no
    // error was encountered the attribute might be invalid, but the type is
    // legit.
    if ((attrType != GraphAttributeType::Invalid) &&
        !attr &&
        inoutput.GetAttr().HasAuthoredValue()) {
        attr = inoutput.GetAttr();
        attrType = isInput ? GraphAttributeType::Input :
                             GraphAttributeType::Output;
    }

    return std::make_pair(attr, attrType);
}

UsdAttribute
GraphInput::GetValueProducingAttribute(GraphAttributeType* attrType) const
{
    TRACE_SCOPE("GraphInput::GetValueProducingAttribute");

    // We track which attributes we've visited so far to avoid getting caught
    // in an infinite loop, if the network contains a cycle.
    _SmallSdfPathVector foundAttributes;

    UsdAttribute attr;
    GraphAttributeType aType;
    std::tie(attr, aType) =
            _GetValueProducingAttributeRecursive(*this, foundAttributes);

    // We track the type of attributes, even if they don't carry a value. But
    // we do not want to return the type if no value was found
    if (!attr)
        aType = GraphAttributeType::Invalid;

    if (attrType)
        *attrType = aType;

    return attr;
}

PXR_NAMESPACE_CLOSE_SCOPE
