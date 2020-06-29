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
#include "./connectableAPI.h"
#include "./utils.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"
#include "pxr/usd/usd/tokens.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<GraphConnectableAPI,
        TfType::Bases< UsdAPISchemaBase > >();
    
}

TF_DEFINE_PRIVATE_TOKENS(
    _schemaTokens,
    (ConnectableAPI)
);

/* virtual */
GraphConnectableAPI::~GraphConnectableAPI()
{
}

/* static */
GraphConnectableAPI
GraphConnectableAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return GraphConnectableAPI();
    }
    return GraphConnectableAPI(stage->GetPrimAtPath(path));
}


/* virtual */
UsdSchemaType GraphConnectableAPI::_GetSchemaType() const {
    return GraphConnectableAPI::schemaType;
}

/* static */
const TfType &
GraphConnectableAPI::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<GraphConnectableAPI>();
    return tfType;
}

/* static */
bool 
GraphConnectableAPI::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
GraphConnectableAPI::_GetTfType() const
{
    return _GetStaticTfType();
}

/*static*/
const TfTokenVector&
GraphConnectableAPI::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames;
    static TfTokenVector allNames =
        UsdAPISchemaBase::GetSchemaAttributeNames(true);

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

#include "pxr/base/tf/envSetting.h"

#include "pxr/usd/pcp/layerStack.h"
#include "pxr/usd/pcp/node.h"
#include "pxr/usd/pcp/primIndex.h"

#include "pxr/usd/sdf/attributeSpec.h"
#include "pxr/usd/sdf/propertySpec.h"
#include "pxr/usd/sdf/relationshipSpec.h"

#include "tokens.h"
#include "input.h"
#include "output.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_ENV_SETTING(
    GRAPH_ENABLE_BIDIRECTIONAL_INTERFACE_CONNECTIONS, false,
    "Enables authoring of connections to interface attributes from shader "
    "inputs (or parameters). This allows multiple connections to the same "
    "interface attribute when authoring shading networks with the old "
    "encoding.");

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (outputName)
    (outputs)
);

/* static */
bool 
GraphConnectableAPI::IsNode() const
{
    return GetPrim().IsA<GraphNode>();
}

bool 
GraphConnectableAPI::IsNodeGraph() const
{
    return GetPrim().IsA<GraphGraph>();
}

/* virtual */
bool 
GraphConnectableAPI::_IsCompatible() const
{
    if (!UsdAPISchemaBase::_IsCompatible() )
        return false;

    // Shaders and node-graphs are compatible with this API schema. 
    // XXX: What if the typeName isn't known (eg, pure over)?
    return IsNode() || IsNodeGraph();
}

static bool 
_CanConnectOutputToSource(const GraphOutput &output, 
                          const UsdAttribute &source,
                          std::string *reason)
{
    if (!output.IsDefined()) {
        if (reason) {
            *reason = TfStringPrintf("Invalid output");
        }
        return false;
    }

    // Only outputs on node-graphs are connectable.
    if (!GraphConnectableAPI(output.GetPrim()).IsNodeGraph()) {
        if (reason) {
            *reason = "Output does not belong to a node-graph.";
        }
        return false;
    }

    if (source) {
        // Ensure that the source prim is a descendent of the node-graph owning 
        // the output.
        const SdfPath sourcePrimPath = source.GetPrim().GetPath();
        const SdfPath outputPrimPath = output.GetPrim().GetPath();

        if (!sourcePrimPath.HasPrefix(outputPrimPath)) {
            if (reason) {
                *reason = TfStringPrintf("Source of output '%s' on node-graph "
                    "at path <%s> is outside the node-graph: <%s>",
                    source.GetName().GetText(), outputPrimPath.GetText(),
                    sourcePrimPath.GetText());
            }
            return false;
        }
        
    }

    return true;
}

bool
GraphConnectableAPI::CanConnect(
    const GraphOutput &output, 
    const UsdAttribute &source)
{
    std::string reason;
    // The reason why a connection can't be made isn't exposed currently.
    // We may want to expose it in the future, especially when we have 
    // validation in USD.
    return _CanConnectOutputToSource(output, source, &reason);
}

bool
_CanConnectInputToSource(const GraphInput &input, 
                         const UsdAttribute &source,
                         std::string *reason)
{
    if (!input.IsDefined()) {
        if (reason) {
            *reason = TfStringPrintf("Invalid input: %s",  
                input.GetAttr().GetPath().GetText());
        }
        return false;
    }

    if (!source) {
        if (reason) {
            *reason = TfStringPrintf("Invalid source: %s", 
                source.GetPath().GetText());
        }
        return false;
    }

    TfToken inputConnectability = input.GetConnectability();
    if (inputConnectability == GraphTokens->full) {
        return true;
    } else if (inputConnectability == GraphTokens->interfaceOnly) {
        if (GraphInput::IsInput(source)) {
            TfToken sourceConnectability = 
                GraphInput(source).GetConnectability();
            if (sourceConnectability == GraphTokens->interfaceOnly) {
                return true;
            }
        }
    }

    if (reason) {
        *reason = TfStringPrintf("Input connectability is 'interfaceOnly' and "
            "source does not have 'interfaceOnly' connectability.");
    }

    return false;
}

bool
GraphConnectableAPI::CanConnect(
    const GraphInput &input, 
    const UsdAttribute &source)
{
    std::string reason;
    // The reason why a connection can't be made isn't exposed currently.
    // We may want to expose it in the future, especially when we have 
    // validation in USD.
    return _CanConnectInputToSource(input, source, &reason);
}

static TfToken 
_GetConnectionRelName(const TfToken &attrName)
{
    return TfToken(GraphTokens->connectedSourceFor.GetString() + 
                   attrName.GetString());
}

static
UsdRelationship 
_GetConnectionRel(
    const UsdProperty &shadingProp, 
    bool create)
{
    // If it's already a relationship, simply return it as-is.
    if (UsdRelationship rel = shadingProp.As<UsdRelationship>())
        return rel;

    const UsdPrim& prim = shadingProp.GetPrim();

    // If it's an attribute, return the associated connectedSourceFor 
    // relationship.
    UsdAttribute shadingAttr = shadingProp.As<UsdAttribute>();
    if (shadingAttr) {    
        const TfToken& relName = _GetConnectionRelName(shadingAttr.GetName());
        if (UsdRelationship rel = prim.GetRelationship(relName)) {
            return rel;
        }
        else if (create) {
            return prim.CreateRelationship(relName, /* custom = */ false);
        }
    }
    
    return UsdRelationship();
}

/* static */
bool  
GraphConnectableAPI::_ConnectToSource(
    UsdProperty const &shadingProp,
    GraphConnectableAPI const &source, 
    TfToken const &sourceName,
    TfToken const &renderTarget,
    GraphAttributeType const sourceType,
    SdfValueTypeName typeName)
{
    UsdPrim sourcePrim = source.GetPrim();
    bool  success = true;

    // XXX it WBN to be able to validate source itself, guaranteeing
    // that the source is, in fact connectable (i.e., a shader or node-graph).
    // However, it remains useful to be able to target a pure-over.
    if (sourcePrim) {
        std::string prefix = GraphUtils::GetPrefixForAttributeType(
            sourceType);
        TfToken sourceAttrName(prefix + sourceName.GetString());

        UsdAttribute sourceAttr = sourcePrim.GetAttribute(sourceAttrName);
        if (!sourceAttr) {
            // If the sourcePrim contains a relationship with the source 
            // name, then it must be a terminal output on a node-graph and 
            // cannot be connected to.
            if (sourcePrim.GetRelationship(sourceAttrName)) {
                TF_CODING_ERROR("Cannot connect shading property <%s>'s source"
                    "to existing relationship '%s' on source prim <%s>..",
                    shadingProp.GetName().GetText(),
                    sourceAttrName.GetText(), 
                    sourcePrim.GetPath().GetText());
                return false;
            }
        }

        // If a typeName isn't specified, 
        if (!typeName) {
            // If shadingProp is not an attribute, it must be a terminal output 
            // on a node-graph. Hence wrapping shadingProp in a GraphOutput 
            // and asking for its typeName should give us the desired answer.
            typeName = GraphOutput(shadingProp).GetTypeName();
        }

        // First make sure there is a source attribute of the proper type
        // on the sourcePrim.
        if (!sourceAttr) {
            sourceAttr = sourcePrim.CreateAttribute(sourceAttrName, typeName,
                /* custom = */ false);
        }
        
        UsdAttribute shadingAttr = shadingProp.As<UsdAttribute>();
        if (!shadingAttr) {
            TF_CODING_ERROR("Attempted to author a connection on an invalid"
                "shading property <%s>.", UsdDescribe(shadingAttr).c_str());
            return false;
        }
        success = shadingAttr.SetConnections(
            SdfPathVector{sourceAttr.GetPath()});

            
        
    } else if (!source) {
        TF_CODING_ERROR("Failed connecting shading property <%s>. "
                        "The given source shader prim <%s> is not defined", 
                        shadingProp.GetPath().GetText(),
                        source.GetPrim() ? source.GetPath().GetText() :
                        "invalid-prim");
        return false;
    }


    return success;
}

/* static */
bool  
GraphConnectableAPI::ConnectToSource(
    UsdProperty const &shadingProp,
    GraphConnectableAPI const &source, 
    TfToken const &sourceName, 
    GraphAttributeType const sourceType,
    SdfValueTypeName typeName)
{
    return GraphConnectableAPI::_ConnectToSource(shadingProp, source, 
        sourceName, /* renderTarget */ TfToken(), 
        sourceType, typeName);
}

/* static */
bool 
GraphConnectableAPI::ConnectToSource(
    UsdProperty const &shadingProp,
    SdfPath const &sourcePath)
{
    // sourcePath needs to be a property path for us to make a connection.
    if (!sourcePath.IsPropertyPath())
        return false;

    UsdPrim sourcePrim = shadingProp.GetStage()->GetPrimAtPath(
        sourcePath.GetPrimPath());
    GraphConnectableAPI source(sourcePrim);
    // We don't validate GraphConnectableAPI, as the type of the source prim 
    // may be unknown. (i.e. it could be a pure over or a typeless def).

    TfToken sourceName;
    GraphAttributeType sourceType;
    std::tie(sourceName, sourceType) = GraphUtils::GetBaseNameAndType(
        sourcePath.GetNameToken());

    // If shadingProp is not an attribute, it must be a terminal output on a
    // node-graph. Hence wrapping shadingProp in a GraphOutput and asking for 
    // its typeName should give us the desired answer.
    SdfValueTypeName typeName = GraphOutput(shadingProp).GetTypeName();
    return ConnectToSource(shadingProp, source, sourceName, sourceType, 
        typeName);
}

/* static */
bool 
GraphConnectableAPI::ConnectToSource(
    UsdProperty const &shadingProp, 
    GraphInput const &sourceInput) 
{
    return GraphConnectableAPI::_ConnectToSource(shadingProp, sourceInput, 
        /* renderTarget */ TfToken());
}

/* static */
bool 
GraphConnectableAPI::_ConnectToSource(
    UsdProperty const &shadingProp, 
    GraphInput const &sourceInput,
    TfToken const &renderTarget)
{
    // An interface attribute may be wrapped in the GraphInput, hence get the 
    // 
    TfToken sourceName;
    GraphAttributeType sourceType;
    std::tie(sourceName, sourceType) = GraphUtils::GetBaseNameAndType(
        sourceInput.GetFullName());

    return _ConnectToSource(
        shadingProp, 
        GraphConnectableAPI(sourceInput.GetPrim()),
        sourceName, renderTarget,
        sourceType, 
        sourceInput.GetTypeName());
}

/* static */
bool 
GraphConnectableAPI::ConnectToSource(
    UsdProperty const &shadingProp, 
    GraphOutput const &sourceOutput)
{
    if (sourceOutput.IsTerminal()) {
        TF_CODING_ERROR("Cannot connect shading property <%s>'s source to "
            "terminal output '%s'.", shadingProp.GetName().GetText(),
            sourceOutput.GetFullName().GetText());
        return false;
    }

    return GraphConnectableAPI::ConnectToSource(shadingProp, 
        GraphConnectableAPI(sourceOutput.GetPrim()),
        sourceOutput.GetBaseName(), GraphAttributeType::Output,
        sourceOutput.GetTypeName());
}

/* static */
bool
GraphConnectableAPI::GetConnectedSource(
    UsdProperty const &shadingProp,
    GraphConnectableAPI *source, 
    TfToken *sourceName,
    GraphAttributeType *sourceType)
{
    TRACE_SCOPE("GraphConnectableAPI::GetConnectedSource");

    if (!(source && sourceName && sourceType)) {
        TF_CODING_ERROR("GetConnectedSource() requires non-NULL "
                        "output-parameters.");
        return false;
    }
    
    *source = GraphConnectableAPI();

    SdfPathVector sources;
    if (UsdAttribute shadingAttr = shadingProp.As<UsdAttribute>()) {
        shadingAttr.GetConnections(&sources);
    }

    // XXX(validation)  sources.size() <= 1, also sourceName,
    //                  target Material == source Material ?
    if (sources.size() == 1) {
        SdfPath const & path = sources[0];
        UsdObject target = shadingProp.GetStage()->GetObjectAtPath(path);
        *source = GraphConnectableAPI(target.GetPrim());

       if (path.IsPropertyPath()){
            TfToken const &attrName(path.GetNameToken());

            std::tie(*sourceName, *sourceType) = 
                GraphUtils::GetBaseNameAndType(attrName);
            return target.Is<UsdAttribute>();
        } 
    }

    return false;
}

/* static  */
bool 
GraphConnectableAPI::GetRawConnectedSourcePaths(
    UsdProperty const &shadingProp, 
    SdfPathVector *sourcePaths)
{
    if (UsdAttribute shadingAttr = shadingProp.As<UsdAttribute>()) {
        if (!shadingAttr.GetConnections(sourcePaths)) {
            TF_WARN("Unable to get connections for shading attribute <%s>", 
                shadingAttr.GetPath().GetText());
            return false;
        }
    }
    
    return true;
}

/* static */
bool 
GraphConnectableAPI::HasConnectedSource(const UsdProperty &shadingProp)
{
    // This MUST have the same semantics as GetConnectedSource().
    // XXX someday we might make this more efficient through careful
    // refactoring, but safest to just call the exact same code.
    GraphConnectableAPI source;
    TfToken        sourceName;
    GraphAttributeType sourceType;
    return GraphConnectableAPI::GetConnectedSource(shadingProp, 
        &source, &sourceName, &sourceType);
}

// This tests if a given node represents a "live" base material,
// i.e. once that hasn't been "flattened out" due to being
// pulled across a reference to a library.
static bool
_NodeRepresentsLiveBaseMaterial(const PcpNodeRef &node)
{
    bool isLiveBaseMaterial = false;
    for (PcpNodeRef n = node; 
            n; // 0, or false, means we are at the root node
            n = n.GetOriginNode()) {
        switch(n.GetArcType()) {
        case PcpArcTypeSpecialize:
            isLiveBaseMaterial = true;
            break;
        // dakrunch: specializes across references are actually still valid.
        // 
        // case PcpArcTypeReference:
        //     if (isLiveBaseMaterial) {
        //         // Node is within a base material, but that is in turn
        //         // across a reference. That means this is a library
        //         // material, so it is not live and we should flatten it
        //         // out.  Continue iterating, however, since this
        //         // might be referenced into some other live base material
        //         // downstream.
        //         isLiveBaseMaterial = false;
        //     }
        //     break;
        default:
            break;
        }
    }
    return isLiveBaseMaterial;
}

/* static */
bool 
GraphConnectableAPI::IsSourceConnectionFromBaseMaterial(
    const UsdProperty &shadingProp)
{
    if (UsdAttribute shadingAttr = shadingProp.As<UsdAttribute>()) {
        // USD core doesn't provide a UsdResolveInfo style API for asking where
        // connections are authored, so we do it here ourselves.
        // Find the strongest opinion about connections.
        SdfAttributeSpecHandle strongestAttrSpecWithConnections;
        SdfPropertySpecHandleVector propStack = shadingAttr.GetPropertyStack();
        for (const SdfPropertySpecHandle &prop: propStack) {
            if (SdfAttributeSpecHandle attrSpec =
                TfDynamic_cast<SdfAttributeSpecHandle>(prop)) {
                if (attrSpec->HasConnectionPaths()) {
                    strongestAttrSpecWithConnections = attrSpec;
                    break;
                }
            }
        }

        // Find which prim node introduced that opinion.
        if (strongestAttrSpecWithConnections) {
            for(const PcpNodeRef &node:
                shadingProp.GetPrim().GetPrimIndex().GetNodeRange()) {
                if (node.GetPath() == strongestAttrSpecWithConnections->
                        GetPath().GetPrimPath() 
                    &&
                    node.GetLayerStack()->HasLayer(
                        strongestAttrSpecWithConnections->GetLayer())) 
                {
                    return _NodeRepresentsLiveBaseMaterial(node);
                }
            }
        }
        
        
    }
    return false;

}

/* static */
bool 
GraphConnectableAPI::DisconnectSource(UsdProperty const &shadingProp)
{
    bool success = true;
    if (UsdAttribute shadingAttr = shadingProp.As<UsdAttribute>()) {
        success = shadingAttr.BlockConnections();
    }

    return success;
}

/* static */
bool 
GraphConnectableAPI::ClearSource(UsdProperty const &shadingProp)
{
    bool success = true;
    if (UsdAttribute shadingAttr = shadingProp.As<UsdAttribute>()) {
        success = shadingAttr.ClearConnections();
    }
    return success;
}

GraphOutput 
GraphConnectableAPI::CreateOutput(const TfToken& name,
                                     const SdfValueTypeName& typeName) const
{
    return GraphOutput(GetPrim(), name, typeName);
}

GraphOutput 
GraphConnectableAPI::GetOutput(const TfToken &name) const
{
    TfToken outputAttrName(GraphTokens->outputs.GetString() + 
                           name.GetString());
    if (GetPrim().HasAttribute(outputAttrName)) {
        return GraphOutput(GetPrim().GetAttribute(outputAttrName));
    } 

    return GraphOutput();
}

GraphOutput 
GraphConnectableAPI::GetOutput(const int index) const
{
    std::vector<GraphOutput> outputs = GetOutputs();
    if(index >=0 && index < outputs.size())
        return outputs[index];
    return GraphOutput();
}

std::vector<GraphOutput> 
GraphConnectableAPI::GetOutputs() const
{
    std::vector<GraphOutput> ret;

    std::vector<UsdAttribute> attrs = GetPrim().GetAttributes();
    TF_FOR_ALL(attrIter, attrs) { 
        const UsdAttribute& attr = *attrIter;
        // If the attribute is in the "outputs:" namespace, then
        // it must be a valid GraphOutput.
        if (TfStringStartsWith(attr.GetName().GetString(), 
                               GraphTokens->outputs)) {
            ret.push_back(GraphOutput(attr));
        }
    }

    return ret;
}

int 
GraphConnectableAPI::NumOutputs() const
{
    return GetOutputs().size();
}

GraphInput 
GraphConnectableAPI::CreateInput(const TfToken& name,
                                    const SdfValueTypeName& typeName) const
{
    return GraphInput(GetPrim(), name, typeName);
}

GraphInput 
GraphConnectableAPI::GetInput(const TfToken &name) const
{
    TfToken inputAttrName(GraphTokens->inputs.GetString() + 
                          name.GetString());

    if (GetPrim().HasAttribute(inputAttrName)) {
        return GraphInput(GetPrim().GetAttribute(inputAttrName));
    }

    return GraphInput();
}

GraphInput 
GraphConnectableAPI::GetInput(const int index) const
{
    std::vector<GraphInput> inputs = GetInputs();
    if(index >=0 && index < inputs.size())
        return inputs[index];
    return GraphInput();
}

std::vector<GraphInput> 
GraphConnectableAPI::GetInputs() const
{
    std::vector<GraphInput> ret;

    std::vector<UsdAttribute> attrs = GetPrim().GetAttributes();
    TF_FOR_ALL(attrIter, attrs) { 
        const UsdAttribute& attr = *attrIter;
        // If the attribute is in the "inputs:" namespace, then
        // it must be a valid GraphInput.
        if (TfStringStartsWith(attr.GetName().GetString(), 
                               GraphTokens->inputs)) {
            ret.push_back(GraphInput(attr));
            continue;
        }
    }
    return ret;
}

int
GraphConnectableAPI::NumInputs() const
{
    std::vector<GraphInput> ret = GetInputs();
    return ret.size();
}


PXR_NAMESPACE_CLOSE_SCOPE
