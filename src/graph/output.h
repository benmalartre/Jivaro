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
#ifndef Graph_OUTPUT_H
#define Graph_OUTPUT_H

#include "pxr/pxr.h"
#include "api.h"
#include "tokens.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/usd/property.h"
#include "pxr/usd/usd/relationship.h"
#include "pxr/usd/ndr/declare.h"
#include "utils.h"

#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

class GraphConnectableAPI;
class GraphInput;

/// \class GraphOutput
/// 
/// This class encapsulates a shader or node-graph output, which is a 
/// connectable property representing a typed, externally computed value.
/// 
class GraphOutput
{
public:
    /// Default constructor returns an invalid Output.  Exists for 
    /// container classes
    GraphOutput()
    {
        // nothing
    }

    /// Get the name of the attribute associated with the output. 
    /// 
    /// \note Returns the relationship name if it represents a terminal on a 
    /// material.
    /// 
    TfToken const &GetFullName() const { 
        return _prop.GetName(); 
    }

    /// Returns the name of the output. 
    /// 
    /// We call this the base name since it strips off the "outputs:" namespace 
    /// prefix from the attribute name, and returns it.
    /// 
    /// \note This simply returns the full property name if the Output represents a 
    /// terminal on a material.
    /// 
    GRAPH_API
    TfToken GetBaseName() const;

    /// Get the prim that the output belongs to.
    UsdPrim GetPrim() const {
        return _prop.GetPrim();
    }

    /// Get the "scene description" value type name of the attribute associated 
    /// with the output.
    /// 
    /// \note If this is an output belonging to a terminal on a material, which 
    /// does not have an associated attribute, we return 'Token' as the type.
    /// 
    GRAPH_API
    SdfValueTypeName GetTypeName() const;
    
    /// Set a value for the output.
    /// 
    /// It's unusual to be setting a value on an output since it represents 
    /// an externally computed value. The Set API is provided here just for the 
    /// sake of completeness and uniformity with other property schema.
    /// 
    GRAPH_API
    bool Set(const VtValue& value, 
             UsdTimeCode time = UsdTimeCode::Default()) const;

    /// \overload 
    /// Set the attribute value of the Output at \p time 
    /// 
    template <typename T>
    bool Set(const T& value, UsdTimeCode time = UsdTimeCode::Default()) const {
        if (UsdAttribute attr = GetAttr()) {
            return attr.Set(value, time);
        }
        return false;
    }

    /// \name Configuring the Output's Type
    /// @{

    /// Specify an alternative, renderer-specific type to use when
    /// emitting/translating this output, rather than translating based
    /// on its GetTypeName()
    ///
    /// For example, we set the renderType to "struct" for outputs that
    /// are of renderman custom struct types.
    ///
    /// \return true on success
    GRAPH_API
    bool SetRenderType(TfToken const& renderType) const;

    /// Return this output's specialized renderType, or an empty
    /// token if none was authored.
    ///
    /// \sa SetRenderType()
    GRAPH_API
    TfToken GetRenderType() const;

    /// Return true if a renderType has been specified for this
    /// output.
    ///
    /// \sa SetRenderType()
    GRAPH_API
    bool HasRenderType() const;

    /// @}

    // ---------------------------------------------------------------
    /// \name UsdAttribute API
    // ---------------------------------------------------------------

    /// @{

    /// Speculative constructor that will produce a valid GraphOutput when
    /// \p attr already represents a shade Output, and produces an \em invalid 
    /// GraphOutput otherwise (i.e. the explicit bool conversion operator 
    /// will return false).
    GRAPH_API
    explicit GraphOutput(const UsdAttribute &attr);

    /// Test whether a given UsdAttribute represents a valid Output, which
    /// implies that creating a GraphOutput from the attribute will succeed.
    ///
    /// Success implies that \c prop.IsDefined() is true.
    GRAPH_API
    static bool IsOutput(const UsdAttribute &attr);

    /// Explicit UsdAttribute extractor.
    UsdAttribute GetAttr() const { return _prop.As<UsdAttribute>(); }
    
    /// Explicit UsdProperty extractor.
    const UsdProperty &GetProperty() const { return _prop; }

    /// Allow GraphOutput to auto-convert to UsdAttribute, so you can
    /// pass a GraphOutput to any function that accepts a UsdAttribute or
    /// const-ref thereto.
    operator UsdAttribute () const { return GetAttr(); }

    /// Allow GraphOutput to auto-convert to UsdProperty, so you can
    /// pass a GraphOutput to any function that accepts a UsdProperty or
    /// const-ref thereto.
    operator const UsdProperty & () const { return GetProperty(); }

    /// Explicit UsdRelationship extractor.
    UsdRelationship GetRel() const { return _prop.As<UsdRelationship>(); }
    
    /// Returns whether the Output represents a terminal relationship on a 
    /// material, which is a concept we'd like to retire in favor of outputs.
    /// This is temporary convenience API.
    bool IsTerminal() const { 
        return static_cast<bool>(GetRel()); 
    }

    /// Return true if the wrapped UsdAttribute is defined, and in
    /// addition the attribute is identified as an output.
    ///
    /// For backwards compatibility, also returns true for the case
    /// of a valid terminal relationship; see IsTerminal().
    bool IsDefined() const {
        if (UsdAttribute attr = GetAttr()) {
            return IsOutput(attr);
        }
        return IsTerminal();
    }

    /// @}

    // -------------------------------------------------------------------------
    /// \name Connections API
    // -------------------------------------------------------------------------
    /// @{

    /// Determines whether this Output can be connected to the given 
    /// source attribute, which can be an input or an output.
    /// 
    /// An output is considered to be connectable only if it belongs to a 
    /// node-graph. Shader outputs are not connectable.
    /// 
    /// \sa GraphConnectableAPI::CanConnect
    GRAPH_API
    bool CanConnect(const UsdAttribute &source) const;

    /// \overload
    GRAPH_API
    bool CanConnect(const GraphInput &sourceInput) const;

    /// \overload
    GRAPH_API
    bool CanConnect(const GraphOutput &sourceOutput) const;

    /// Authors a connection for this Output to the source described by the 
    /// following three elements: 
    /// \p source, the connectable owning the source,
    /// \p sourceName, the name of the source and 
    /// \p sourceType, the value type of the source shading attribute.
    ///
    /// \p typeName if specified, is the typename of the attribute to create 
    /// on the source if it doesn't exist. It is also used to validate whether 
    /// the types of the source and consumer of the connection are compatible.
    ///
    /// \sa GraphConnectableAPI::ConnectToSource
    ///
    GRAPH_API
    bool ConnectToSource(
        GraphConnectableAPI const &source, 
        TfToken const &sourceName, 
        GraphAttributeType const sourceType=GraphAttributeType::Output,
        SdfValueTypeName typeName=SdfValueTypeName()) const;

    /// Authors a connection for this Output to the source at the given path.
    /// 
    /// \sa GraphConnectableAPI::ConnectToSource
    ///
    GRAPH_API
    bool ConnectToSource(SdfPath const &sourcePath) const;

    /// Connects this Output to the given input, \p sourceInput.
    /// 
    /// \sa GraphConnectableAPI::ConnectToSource
    ///
    GRAPH_API
    bool ConnectToSource(GraphInput const &sourceInput) const;

    /// Connects this Output to the given output, \p sourceOutput.
    /// 
    /// \sa GraphConnectableAPI::ConnectToSource
    ///
    GRAPH_API
    bool ConnectToSource(GraphOutput const &sourceOutput) const;

    /// Finds the source of a connection for this Output.
    /// 
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
    /// \sa GraphConnectableAPI::GetConnectedSource
    ///
    GRAPH_API
    bool GetConnectedSource(GraphConnectableAPI *source, 
                            TfToken *sourceName,
                            GraphAttributeType *sourceType) const;

    /// Returns the "raw" (authored) connected source paths for this Output.
    /// 
    /// \sa GraphConnectableAPI::GetRawConnectedSourcePaths
    ///
    GRAPH_API
    bool GetRawConnectedSourcePaths(SdfPathVector *sourcePaths) const;

    /// Returns true if and only if this Output is currently connected to a 
    /// valid (defined) source. 
    ///
    /// \sa GraphConnectableAPI::HasConnectedSource
    /// 
    GRAPH_API
    bool HasConnectedSource() const;

    /// Returns true if the connection to this Output's source, as returned by 
    /// GetConnectedSource(), is authored across a specializes arc, which is 
    /// used to denote a base material.
    /// 
    /// \sa GraphConnectableAPI::IsSourceConnectionFromBaseMaterial
    ///
    GRAPH_API
    bool IsSourceConnectionFromBaseMaterial() const;

    /// Disconnect source for this Output.
    /// 
    /// \sa GraphConnectableAPI::DisconnectSource
    ///
    GRAPH_API
    bool DisconnectSource() const;

    /// Clears source for this shading property in the current UsdEditTarget.
    ///
    /// Most of the time, what you probably want is DisconnectSource()
    /// rather than this function.
    ///
    /// \sa GraphConnectableAPI::ClearSource
    ///
    GRAPH_API
    bool ClearSource() const;

    /// @}

    /// Return true if this Output is valid for querying and authoring
    /// values and metadata, which is identically equivalent to IsDefined().
    explicit operator bool() const { 
        return IsDefined(); 
    }

    /// Equality comparison. Returns true if \a lhs and \a rhs represent the 
    /// same GraphOutput, false otherwise.
    friend bool operator==(const GraphOutput &lhs, const GraphOutput &rhs) {
        return lhs.GetProperty() == rhs.GetProperty();
    }

private:
    friend class GraphConnectableAPI;

    // Befriend UsdRiMaterialAPI which will provide a backwards compatible 
    // interface for managing terminal relationships, which turn into outputs
    // in the new encoding of shading networks.
    // This is temporary to assist in the transition to the new shading 
    // encoding.
    friend class UsdRiMaterialAPI;

    // Constructor that creates a GraphOutput with the given name on the 
    // given prim.
    // \p name here is the unnamespaced name of the output.
    GraphOutput(UsdPrim prim,
                   TfToken const &name,
                   SdfValueTypeName const &typeName);

    // Speculative constructor that will produce a valid GraphOutput when 
    // \p rel represents a terminal relationship on a material, a concept that 
    // has been retired in favor of outputs represented as (attribute, 
    // relationship) pair.
    // 
    // Outputs wrapping a terminal relationship are always considered valid 
    // as long as the relationship is defined and valid.
    // 
    // This exists only to allow higher level API to be backwards compatible
    // and treat terminals and outputs uniformly.
    // 
    GRAPH_API
    explicit GraphOutput(const UsdRelationship &rel);

    // Constructor that wraps the given shading property in a GraphOutput
    // object.
    explicit GraphOutput(const UsdProperty &prop);

    // This is currently a relationship if the output belongs to a node-graph.
    // In the future, all outputs will have associated attributes and we 
    // can switch this to be a UsdAttribute instead of UsdProperty.
    UsdProperty _prop;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif // Graph_OUTPUT_H
