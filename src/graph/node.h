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
#ifndef GRAPH_GENERATED_NODE_H
#define GRAPH_GENERATED_NODE_H

/// \file Graph/node.h

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/usd/usd/typed.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "./tokens.h"

#include "input.h"
#include "output.h"
#include "pxr/usd/ndr/declare.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// NODE                                                                       //
// -------------------------------------------------------------------------- //

/// \class GraphNode
///
/// Base class for all Amnesia Nodes.
/// 
///
class GraphNode : public UsdTyped
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaType
    static const UsdSchemaType schemaType = UsdSchemaType::ConcreteTyped;

    /// Construct a GraphNode on UsdPrim \p prim .
    /// Equivalent to GraphNode::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit GraphNode(const UsdPrim& prim=UsdPrim())
        : UsdTyped(prim)
    {
    }

    /// Construct a GraphNode on the prim held by \p schemaObj .
    /// Should be preferred over GraphNode(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit GraphNode(const UsdSchemaBase& schemaObj)
        : UsdTyped(schemaObj)
    {
    }

    /// Destructor.
    GRAPH_API
    virtual ~GraphNode();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    GRAPH_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a GraphNode holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// GraphNode(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    GRAPH_API
    static GraphNode
    Get(const UsdStagePtr &stage, const SdfPath &path);

    /// Attempt to ensure a \a UsdPrim adhering to this schema at \p path
    /// is defined (according to UsdPrim::IsDefined()) on this stage.
    ///
    /// If a prim adhering to this schema at \p path is already defined on this
    /// stage, return that prim.  Otherwise author an \a SdfPrimSpec with
    /// \a specifier == \a SdfSpecifierDef and this schema's prim type name for
    /// the prim at \p path at the current EditTarget.  Author \a SdfPrimSpec s
    /// with \p specifier == \a SdfSpecifierDef and empty typeName at the
    /// current EditTarget for any nonexistent, or existing but not \a Defined
    /// ancestors.
    ///
    /// The given \a path must be an absolute prim path that does not contain
    /// any variant selections.
    ///
    /// If it is impossible to author any of the necessary PrimSpecs, (for
    /// example, in case \a path cannot map to the current UsdEditTarget's
    /// namespace) issue an error and return an invalid \a UsdPrim.
    ///
    /// Note that this method may return a defined prim whose typeName does not
    /// specify this schema class, in case a stronger typeName opinion overrides
    /// the opinion at the current EditTarget.
    ///
    GRAPH_API
    static GraphNode
    Define(const UsdStagePtr &stage, const SdfPath &path);

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
    
    /// Constructor that takes a ConnectableAPI object.
    /// Allow implicit (auto) conversion of GraphNode to 
    /// GraphConnectableAPI, so that a shader can be passed into any function
    /// that accepts a ConnectableAPI.
    GRAPH_API
    GraphNode(const GraphConnectableAPI &connectable);

    /// Contructs and returns a GraphConnectableAPI object with this shader.
    ///
    /// Note that most tasks can be accomplished without explicitly constructing 
    /// a GraphConnectable API, since connection-related API such as
    /// GraphConnectableAPI::ConnectToSource() are static methods, and 
    /// GraphNode will auto-convert to a GraphConnectableAPI when 
    /// passed to functions that want to act generically on a connectable
    /// GraphConnectableAPI object.
    GRAPH_API
    GraphConnectableAPI ConnectableAPI() const;

    // -------------------------------------------------------------------------
    /// \name Outputs API
    ///
    /// Outputs represent a typed property on a shader or node-graph whose value 
    /// is computed externally. 
    /// 
    /// When they exist on a node-graph, they are connectable and are typically 
    /// connected to the output of a shader within the node-graph.
    /// 
    /// @{
        
    /// Create an output which can either have a value or can be connected.
    /// The attribute representing the output is created in the "outputs:" 
    /// namespace. Outputs on a shader cannot be connected, as their 
    /// value is assumed to be computed externally.
    /// 
    GRAPH_API
    GraphOutput CreateOutput(const TfToken& name,
                                const SdfValueTypeName& typeName);

    /// Return the requested output if it exists.
    /// 
    GRAPH_API
    GraphOutput GetOutput(const TfToken &name) const;

    /// Outputs are represented by attributes in the "outputs:" namespace.
    /// 
    GRAPH_API
    std::vector<GraphOutput> GetOutputs() const;

    /// @}

    // ------------------------------------------------------------------------- 

    /// \name Inputs API
    ///
    /// Inputs are connectable properties with a typed value. 
    /// 
    /// On shaders, the shader parameters are encoded as inputs. On node-graphs,
    /// interface attributes are represented as inputs.
    /// 
    /// @{
        
    /// Create an input which can either have a value or can be connected.
    /// The attribute representing the input is created in the "inputs:" 
    /// namespace. Inputs on both shaders and node-graphs are connectable.
    /// 
    GRAPH_API
    GraphInput CreateInput(const TfToken& name,
                              const SdfValueTypeName& typeName);

    /// Return the requested input if it exists.
    /// 
    GRAPH_API
    GraphInput GetInput(const TfToken &name) const;

    /// Inputs are represented by attributes in the "inputs:" namespace.
    /// 
    GRAPH_API
    std::vector<GraphInput> GetInputs() const;

    /// @}

    // ------------------------------------------------------------------------
    
    /// \anchor GraphNode_SdrMetadata_API
    /// \name Shader Sdr Metadata API
    /// 
    /// This section provides API for authoring and querying shader registry
    /// metadata. When the shader's implementationSource is <b>sourceAsset</b> 
    /// or <b>sourceCode</b>, the authored "sdrMetadata" dictionary value 
    /// provides additional metadata needed to process the shader source
    /// correctly. It is used in combination with the sourceAsset or sourceCode
    /// value to fetch the appropriate node from the shader registry.
    /// 
    /// We expect the keys in sdrMetadata to correspond to the keys 
    /// in \ref SdrNodeMetadata. However, this is not strictly enforced in the 
    /// API. The only allowed value type in the "sdrMetadata" dictionary is a 
    /// std::string since it needs to be converted into a NdrTokenMap, which Sdr
    /// will parse using the utilities available in \ref SdrMetadataHelpers.
    /// 
    /// @{

    /// Returns this shader's composed "sdrMetadata" dictionary as a 
    /// NdrTokenMap.
    GRAPH_API
    NdrTokenMap GetSdrMetadata() const;
    
    /// Returns the value corresponding to \p key in the composed 
    /// <b>sdrMetadata</b> dictionary.
    GRAPH_API
    std::string GetSdrMetadataByKey(const TfToken &key) const;
        
    /// Authors the given \p sdrMetadata on this shader at the current 
    /// EditTarget.
    GRAPH_API
    void SetSdrMetadata(const NdrTokenMap &sdrMetadata) const;

    /// Sets the value corresponding to \p key to the given string \p value, in 
    /// the shader's "sdrMetadata" dictionary at the current EditTarget.
    GRAPH_API
    void SetSdrMetadataByKey(
        const TfToken &key, 
        const std::string &value) const;

    /// Returns true if the shader has a non-empty composed "sdrMetadata" 
    /// dictionary value.
    GRAPH_API
    bool HasSdrMetadata() const;

    /// Returns true if there is a value corresponding to the given \p key in 
    /// the composed "sdrMetadata" dictionary.
    GRAPH_API
    bool HasSdrMetadataByKey(const TfToken &key) const;

    /// Clears any "sdrMetadata" value authored on the shader in the current 
    /// EditTarget.
    GRAPH_API
    void ClearSdrMetadata() const;

    /// Clears the entry corresponding to the given \p key in the 
    /// "sdrMetadata" dictionary authored in the current EditTarget.
    GRAPH_API
    void ClearSdrMetadataByKey(const TfToken &key) const;

    /// @}

    /*
    /// This method attempts to ensure that there is a ShaderNode in the shader 
    /// registry (i.e. \ref SdrRegistry) representing this shader for the 
    /// given \p sourceType. It may return a null pointer if none could be 
    /// found or created.
    GRAPH_API
    SdrShaderNodeConstPtr GetNodeForSourceType(const TfToken &sourceType) 
        const; 
        */
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
