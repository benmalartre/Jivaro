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
#ifndef GRAPH_GENERATED_CONNECTABLEAPI_H
#define GRAPH_GENERATED_CONNECTABLEAPI_H

/// \file Graph/connectableAPI.h

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/usd/usd/apiSchemaBase.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"

#include "node.h"
#include "graph.h"


#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// CONNECTABLEAPI                                                             //
// -------------------------------------------------------------------------- //

/// \class GraphConnectableAPI
///
/// ConnectableAPI is an API schema that provides a common
/// interface for creating outputs and making connections between nodes.
/// The interface is common to all Graph schemas including Node, Graph, 
/// and Deformable.
/// 
/// One can construct a ConnectableAPI directly from a UsdPrim, or
/// from objects of any of the schema classes listed above.  If it seems
/// onerous to need to construct a secondary schema object to interact with
/// Inputs and Outputs, keep in mind that any function whose purpose is either
/// to walk shot description networks via their connections, or to create such
/// networks, can typically be written entirely in terms of 
/// ConnectableAPI objects, without needing to care what the underlying
/// prim type is.
/// 
/// Additionally, the most common ConnectableAPI behaviors
/// (creating Inputs and Outputs, and making connections) are wrapped as
/// convenience methods on the prim schema classes (creation) and 
/// InputPort and OutputPort.
/// 
///
class GraphConnectableAPI : public UsdAPISchemaBase
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaType
    static const UsdSchemaType schemaType = UsdSchemaType::NonAppliedAPI;

    /// Construct a GraphConnectableAPI on UsdPrim \p prim .
    /// Equivalent to GraphConnectableAPI::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit GraphConnectableAPI(const UsdPrim& prim=UsdPrim())
        : UsdAPISchemaBase(prim)
    {
    }

    /// Construct a GraphConnectableAPI on the prim held by \p schemaObj .
    /// Should be preferred over GraphConnectableAPI(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit GraphConnectableAPI(const UsdSchemaBase& schemaObj)
        : UsdAPISchemaBase(schemaObj)
    {
    }

    /// Destructor.
    GRAPH_API
    virtual ~GraphConnectableAPI();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    GRAPH_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a GraphConnectableAPI holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// GraphConnectableAPI(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    GRAPH_API
    static GraphConnectableAPI
    Get(const UsdStagePtr &stage, const SdfPath &path);


protected:
    /// Returns the type of schema this class belongs to.
    ///
    /// \sa UsdSchemaType
    GRAPH_API
    UsdSchemaType _GetSchemaType() const override;

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    GRAPH_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    GRAPH_API
    const TfType &_GetTfType() const override;

public:
    // ===================================================================== //
    // Feel free to add custom code below this line, it will be preserved by 
    // the code generator. 
    //
    // Just remember to: 
    //  - Close the class declaration with }; 
    //  - Close the namespace with PXR_NAMESPACE_CLOSE_SCOPE
    //  - Close the include guard with #endif
    // ===================================================================== //
    // --(BEGIN CUSTOM CODE)--


    protected:
    /// Returns true if the given prim is compatible with this API schema,
    /// i.e. if it is a valid shader or a node-graph.
    GRAPH_API
    bool _IsCompatible() const override;
    
public:

    /// Constructor that takes a GraphNode.
    /// Allow implicit (auto) conversion of GraphNode to 
    /// GraphConnectableAPI, so that a node can be passed into any function
    /// that accepts a ConnectableAPI.
    GraphConnectableAPI(const GraphNode &node);

    /// Constructor that takes a GraphGraph.
    /// Allow implicit (auto) conversion of GraphGraph to 
    /// GraphConnectableAPI, so that a nodegraph can be passed into any function
    /// that accepts a ConnectableAPI.
    GraphConnectableAPI(const GraphGraph &nodeGraph);

    /// Returns true if the prim is a node.
    GRAPH_API
    bool IsNode() const;

    /// Returns true if the prim is a node-graph.
    GRAPH_API
    bool IsNodeGraph() const;

    /// \name Connections 
    /// 
    /// Inputs and outputs on shaders and node-graphs are connectable.
    /// This section provides API for authoring and managing these connections
    /// in a shading network.
    /// 
    /// @{

    /// Determines whether the given input can be connected to the given 
    /// source attribute, which can be an input or an output.
    /// 
    /// The result depends on the "connectability" of the input and the source 
    /// attributes. 
    /// 
    /// \sa GraphInput::SetConnectability
    /// \sa GraphInput::GetConnectability
    GRAPH_API
    static bool CanConnect(const GraphInput &input, 
                           const UsdAttribute &source);

    /// \overload
    GRAPH_API
    static bool CanConnect(const GraphInput &input, 
                           const GraphInput &sourceInput) {
        return CanConnect(input, sourceInput.GetAttr());
    }

    /// \overload
    GRAPH_API
    static bool CanConnect(const GraphInput &input, 
                           const GraphOutput &sourceOutput) {
        return CanConnect(input, sourceOutput.GetAttr());
    }

    /// Determines whether the given output can be connected to the given 
    /// source attribute, which can be an input or an output.
    /// 
    /// An output is considered to be connectable only if it belongs to a 
    /// node-graph. Shader outputs are not connectable.
    /// 
    /// \p source is an optional argument. If a valid UsdAttribute is supplied
    /// for it, this method will return true only if the source attribute is 
    /// owned by a descendant of the node-graph owning the output.
    ///
    GRAPH_API
    static bool CanConnect(const GraphOutput &output, 
                           const UsdAttribute &source=UsdAttribute());

    /// \overload
    GRAPH_API
    static bool CanConnect(const GraphOutput &output, 
                           const GraphInput &sourceInput) {
        return CanConnect(output, sourceInput.GetAttr());
    }

    /// \overload
    GRAPH_API
    static bool CanConnect(const GraphOutput &output, 
                           const GraphOutput &sourceOutput) {
        return CanConnect(output, sourceOutput.GetAttr());
    }

    /// Authors a connection for a given shading property \p shadingProp. 
    /// 
    /// \p shadingProp can represent a parameter, an interface attribute, an
    /// input or an output.
    /// \p sourceName is the name of the shading property that is the target
    /// of the connection. This excludes any namespace prefix that determines 
    /// the type of the source (eg, output or interface attribute).
    /// \p sourceType is used to indicate the type of the shading property 
    /// that is the target of the connection. The source type is used to 
    /// determine the namespace prefix that must be attached to \p sourceName
    /// to determine the source full property name.
    /// \p typeName if specified, is the typename of the attribute to create 
    /// on the source if it doesn't exist. It is also used to validate whether 
    /// the types of the source and consumer of the connection are compatible.
    /// \p source is the connectable prim that produces or contains a value 
    /// for the given shading property.
    /// 
    /// \return 
    /// \c true if a connection was created successfully. 
    /// \c false if \p shadingProp or \p source is invalid.
    /// 
    /// \note This method does not verify the connectability of the shading
    /// property to the source. Clients must invoke CanConnect() themselves
    /// to ensure compatibility.
    /// \note The source shading property is created if it doesn't exist 
    /// already.
    ///
    GRAPH_API
    static bool ConnectToSource(
        UsdProperty const &shadingProp,
        GraphConnectableAPI const &source, 
        TfToken const &sourceName, 
        GraphAttributeType const sourceType=GraphAttributeType::Output,
        SdfValueTypeName typeName=SdfValueTypeName());

    /// \overload
    GRAPH_API
    static bool ConnectToSource(
        GraphInput const &input,
        GraphConnectableAPI const &source, 
        TfToken const &sourceName, 
        GraphAttributeType const sourceType=GraphAttributeType::Output,
        SdfValueTypeName typeName=SdfValueTypeName()) 
    {
        return ConnectToSource(input.GetAttr(), source, sourceName, sourceType, 
            typeName);
    }

    /// \overload
    GRAPH_API
    static bool ConnectToSource(
        GraphOutput const &output,
        GraphConnectableAPI const &source, 
        TfToken const &sourceName, 
        GraphAttributeType const sourceType=GraphAttributeType::Output,
        SdfValueTypeName typeName=SdfValueTypeName()) 
    {
        return ConnectToSource(output.GetProperty(), source, sourceName, sourceType, 
            typeName);
    }

    /// \overload
    /// 
    /// Connect the given shading property to the source at path, \p sourcePath. 
    /// 
    /// \p sourcePath should be the fully namespaced property path. 
    /// 
    /// This overload is provided for convenience, for use in contexts where 
    /// the prim types are unknown or unavailable.
    /// 
    GRAPH_API
    static bool ConnectToSource(UsdProperty const &shadingProp, 
                                SdfPath const &sourcePath);

    /// \overload
    GRAPH_API
    static bool ConnectToSource(GraphInput const &input, 
                                SdfPath const &sourcePath) {
        return ConnectToSource(input.GetAttr(), sourcePath);
    }

    /// \overload
    GRAPH_API
    static bool ConnectToSource(GraphOutput const &output, 
                                SdfPath const &sourcePath) {
        return ConnectToSource(output.GetProperty(), sourcePath);
    }

    /// \overload 
    /// 
    /// Connect the given shading property to the given source input. 
    /// 
    GRAPH_API
    static bool ConnectToSource(UsdProperty const &shadingProp, 
                                GraphInput const &sourceInput);

    /// \overload
    GRAPH_API
    static bool ConnectToSource(GraphInput const &input, 
                                GraphInput const &sourceInput) {
        return ConnectToSource(input.GetAttr(), sourceInput);
    }

    /// \overload
    GRAPH_API
    static bool ConnectToSource(GraphOutput const &output, 
                                GraphInput const &sourceInput) {
        return ConnectToSource(output.GetProperty(), sourceInput);                                
    }

    /// \overload 
    /// 
    /// Connect the given shading property to the given source output. 
    /// 
    GRAPH_API
    static bool ConnectToSource(UsdProperty const &shadingProp, 
                                GraphOutput const &sourceOutput);

    /// \overload 
    GRAPH_API
    static bool ConnectToSource(GraphInput const &input, 
                                GraphOutput const &sourceOutput) {
        return ConnectToSource(input.GetAttr(), sourceOutput);                            
    }

    /// \overload 
    GRAPH_API
    static bool ConnectToSource(GraphOutput const &output, 
                                GraphOutput const &sourceOutput) {
        return ConnectToSource(output.GetProperty(), sourceOutput);
    }

private:
    /// \deprecated 
    /// Provided for use by UsdRiLookAPI and UsdRiMaterialAPI to author 
    /// old-style interface attribute connections, which require the 
    /// \p renderTarget argument. 
    /// 
    static bool _ConnectToSource(
        UsdProperty const &shadingProp,
        GraphConnectableAPI const &source, 
        TfToken const &sourceName, 
        TfToken const &renderTarget,
        GraphAttributeType const sourceType=GraphAttributeType::Output,
        SdfValueTypeName typeName=SdfValueTypeName());

protected:
    // Befriend UsdRiLookAPI and UsdRiMaterialAPI temporarily to assist in the
    // transition to the new shading encoding.
    friend class UsdRiLookAPI;
    friend class UsdRiMaterialAPI;
    
    /// \deprecated
    /// Connect the given shading property to the given source input. 
    /// 
    /// Provided for use by UsdRiLookAPI and UsdRiMaterialAPI to author 
    /// old-style interface attribute connections, which require the 
    /// \p renderTarget argument. 
    /// 
    GRAPH_API
    static bool _ConnectToSource(UsdProperty const &shadingProp, 
                                GraphInput const &sourceInput,
                                TfToken const &renderTarget);
    
public:

    /// Finds the source of a connection for the given shading property.
    /// 
    /// \p shadingProp is the shading attribute qhose connection we want to
    /// interrogate.
    /// \p source is an output parameter which will be set to the source 
    /// connectable prim.
    /// \p sourceName will be set to the name of the source shading attribute, 
    /// which may be an input or an output, as specified by \p sourceType
    /// \p sourceType will have the type of the source shading property, i.e.
    /// whether it is an \c Input or \c Output
    ///
    /// \return 
    /// \c true if the shading property is connected to a valid, defined source
    /// attribute.
    /// \c false if the shading property is not connected to a single, defined 
    /// source attribute. 
    /// 
    /// \note The python wrapping for this method returns a 
    /// (source, sourceName, sourceType) tuple if the parameter is connected, 
    /// else \c None
    ///
    GRAPH_API
    static bool GetConnectedSource(UsdProperty const &shadingProp,
                                   GraphConnectableAPI *source, 
                                   TfToken *sourceName,
                                   GraphAttributeType *sourceType);

    /// \overload
    GRAPH_API
    static bool GetConnectedSource(GraphInput const &input,
                                   GraphConnectableAPI *source, 
                                   TfToken *sourceName,
                                   GraphAttributeType *sourceType) {
        return GetConnectedSource(input.GetAttr(), source, sourceName, 
                                  sourceType);
    }

    /// \overload
    GRAPH_API
    static bool GetConnectedSource(GraphOutput const &output,
                                   GraphConnectableAPI *source, 
                                   TfToken *sourceName,
                                   GraphAttributeType *sourceType) {
        return GetConnectedSource(output.GetProperty(), source, sourceName, 
                                  sourceType);
    }

    /// Returns the "raw" (authored) connected source paths for the given 
    /// shading property.
    /// 
    GRAPH_API
    static bool GetRawConnectedSourcePaths(UsdProperty const &shadingProp, 
                                           SdfPathVector *sourcePaths);

    /// \overload
    GRAPH_API
    static bool GetRawConnectedSourcePaths(GraphInput const &input, 
                                           SdfPathVector *sourcePaths) {
        return GetRawConnectedSourcePaths(input.GetAttr(), sourcePaths);
    }

    /// \overload
    GRAPH_API
    static bool GetRawConnectedSourcePaths(GraphOutput const &output, 
                                           SdfPathVector *sourcePaths) {
        return GetRawConnectedSourcePaths(output.GetProperty(), sourcePaths);
    }

    /// Returns true if and only if the shading property is currently connected 
    /// to a valid (defined) source. 
    ///
    /// If you will be calling GetConnectedSource() afterwards anyways, 
    /// it will be \em much faster to instead guard like so:
    /// \code
    /// if (GraphConnectableAPI::GetConnectedSource(property, &source, 
    ///         &sourceName, &sourceType)){
    ///      // process connected property
    /// } else {
    ///      // process unconnected property
    /// }
    /// \endcode
    GRAPH_API
    static bool HasConnectedSource(const UsdProperty &shadingProp);

    /// \overload
    GRAPH_API
    static bool HasConnectedSource(const GraphInput &input) {
        return HasConnectedSource(input.GetAttr());
    }

    /// \overload
    GRAPH_API
    static bool HasConnectedSource(const GraphOutput &output) {
        return HasConnectedSource(output.GetProperty());
    }

    /// Returns true if the connection to the given shading property's source, 
    /// as returned by GraphConnectableAPI::GetConnectedSource(), is authored
    /// across a specializes arc, which is used to denote a base material.
    /// 
    GRAPH_API
    static bool IsSourceConnectionFromBaseMaterial(
        const UsdProperty &shadingProp);

    /// \overload
    GRAPH_API
    static bool IsSourceConnectionFromBaseMaterial(const GraphInput &input) {
        return IsSourceConnectionFromBaseMaterial(input.GetAttr());
    }

    /// \overload
    GRAPH_API
    static bool IsSourceConnectionFromBaseMaterial(const GraphOutput &output) 
    {
        return IsSourceConnectionFromBaseMaterial(output.GetProperty());
    }

    /// Disconnect source for this shading property.
    ///
    /// This may author more scene description than you might expect - we define
    /// the behavior of disconnect to be that, even if a shading property 
    /// becomes connected in a weaker layer than the current UsdEditTarget, the
    /// property will \em still be disconnected in the composition, therefore
    /// we must "block" it (see for e.g. UsdRelationship::BlockTargets()) in
    /// the current UsdEditTarget. 
    ///
    /// \sa ConnectToSource().
    GRAPH_API
    static bool DisconnectSource(UsdProperty const &shadingProp);

    /// \overload
    GRAPH_API
    static bool DisconnectSource(GraphInput const &input) {
        return DisconnectSource(input.GetAttr());
    }

    /// \overload
    GRAPH_API
    static bool DisconnectSource(GraphOutput const &output) {
        return DisconnectSource(output.GetProperty());
    }

    /// Clears source for this shading property in the current UsdEditTarget.
    ///
    /// Most of the time, what you probably want is DisconnectSource()
    /// rather than this function.
    ///
    /// \sa DisconnectSource()
    GRAPH_API
    static bool ClearSource(UsdProperty const &shadingProp);

    /// \overload 
    GRAPH_API
    static bool ClearSource(GraphInput const &input) {
        return ClearSource(input.GetAttr());
    }

    /// \overload 
    GRAPH_API
    static bool ClearSource(GraphOutput const &output) {
        return ClearSource(output.GetProperty());
    }

    /// \deprecated
    /// 
    /// Returns whether authoring of bidirectional connections for the old-style 
    /// interface attributes is enabled. When this returns true, interface 
    /// attribute connections are authored both ways (using both 
    /// interfaceRecipientOf: and connectedSourceFor: relationships)
    /// 
    /// \note This method exists only for testing equality of the old and new
    /// encoding of shading networks in USD. 
    GRAPH_API
    static bool AreBidirectionalInterfaceConnectionsEnabled();

    /// @}


    /// \name Outputs 
    /// @{

    /// Create an output, which represents and externally computed, typed value.
    /// Outputs on node-graphs can be connected. 
    /// 
    /// The attribute representing an output is created in the "outputs:" 
    /// namespace.
    /// 
    GRAPH_API
    GraphOutput CreateOutput(const TfToken& name,
                                const SdfValueTypeName& typeName) const;

    /// Return the requested output if it exists.
    /// 
    /// \p name is the unnamespaced base name.
    ///
    GRAPH_API
    GraphOutput GetOutput(const TfToken &name) const;

    /// Return the requested output if it exists.
    /// 
    /// \p index is the port index.
    ///
    GRAPH_API
    GraphOutput GetOutput(const int index) const;

    /// Returns all outputs on the connectable prim (i.e. node or node-graph). 
    /// Outputs are represented by attributes in the "outputs:" namespace.
    /// 
    GRAPH_API
    std::vector<GraphOutput> GetOutputs() const;

    /// Returns num outputs on the connectable prim (i.e. node or node-graph). 
    /// 
    GRAPH_API
    int NumOutputs() const;

    /// @}

    /// \name Inputs 
    /// @{
        
    /// Create an input which can both have a value and be connected.
    /// The attribute representing the input is created in the "inputs:" 
    /// namespace.
    /// 
    GRAPH_API
    GraphInput CreateInput(const TfToken& name,
                               const SdfValueTypeName& typeName) const;

    /// Return the requested input if it exists.
    /// 
    /// \p name is the unnamespaced base name.
    /// 
    GRAPH_API
    GraphInput GetInput(const TfToken &name) const;

    /// Return the requested input if it exists.
    /// 
    /// \p index is the port number.
    /// 
    GRAPH_API
    GraphInput GetInput(const int index) const;

    /// Returns all inputs on the connectable prim (i.e. node or node-graph). 
    /// Inputs are represented by attributes in the "inputs:" namespace.
    /// 
    GRAPH_API
    std::vector<GraphInput> GetInputs() const;

    /// Returns num inputs on the connectable prim (i.e. node or node-graph). 
    /// 
    GRAPH_API
    int NumInputs() const;

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
