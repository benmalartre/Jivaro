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
#ifndef GRAPH_INPUT_H
#define GRAPH_INPUT_H

#include "pxr/pxr.h"
#include "api.h"
#include "pxr/usd/usd/attribute.h"

#include "pxr/usd/ndr/declare.h"
#include "utils.h"
#include "tokens.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

class GraphConnectableAPI;
class GraphOutput;

/// \class GraphInput
/// 
/// This class encapsulates a node or node-graph input, which is a 
/// connectable property representing a typed value.
/// 
class GraphInput
{
public:
    /// Default constructor returns an invalid Input.  Exists for the sake of
    /// container classes
    GraphInput()
    {
        // nothing
    }

    /// Get the name of the attribute associated with the Input. 
    /// 
    TfToken const &GetFullName() const { 
        return _attr.GetName(); 
    }

    /// Returns the name of the input. 
    /// 
    /// We call this the base name since it strips off the "inputs:" namespace 
    /// prefix from the attribute name, and returns it.
    /// 
    GRAPH_API_H
    TfToken GetBaseName() const;

    /// Get the "scene description" value type name of the attribute associated 
    /// with the Input.
    /// 
    GRAPH_API_H
    SdfValueTypeName GetTypeName() const;
    
    /// Get the prim that the input belongs to.
    UsdPrim GetPrim() const {
        return _attr.GetPrim();
    }

    /// Convenience wrapper for the templated UsdAttribute::Get().
    template <typename T>
    bool Get(T* value, UsdTimeCode time = UsdTimeCode::Default()) const {
        return GetAttr().Get(value, time);
    }

    /// Convenience wrapper for VtValue version of UsdAttribute::Get().
    GRAPH_API_H
    bool Get(VtValue* value, UsdTimeCode time = UsdTimeCode::Default()) const;

    /// Set a value for the Input at \p time.
    /// 
    GRAPH_API_H
    bool Set(const VtValue& value, 
             UsdTimeCode time = UsdTimeCode::Default()) const;

    /// \overload 
    /// Set a value of the Input at \p time.
    /// 
    template <typename T>
    bool Set(const T& value, UsdTimeCode time = UsdTimeCode::Default()) const {
        return _attr.Set(value, time);
    }

    /// Hash functor.
    struct Hash {
        inline size_t operator()(const GraphInput &input) const {
            return hash_value(input._attr);
        }
    };

    /// \name Configuring the Input's Type
    /// @{

    /// Specify an alternative, renderer-specific type to use when
    /// emitting/translating this Input, rather than translating based
    /// on its GetTypeName()
    ///
    /// For example, we set the renderType to "struct" for Inputs that
    /// are of renderman custom struct types.
    ///
    /// \return true on success.
    ///
    GRAPH_API_H
    bool SetRenderType(TfToken const& renderType) const;

    /// Return this Input's specialized renderType, or an empty
    /// token if none was authored.
    ///
    /// \sa SetRenderType()
    GRAPH_API_H
    TfToken GetRenderType() const;

    /// Return true if a renderType has been specified for this Input.
    ///
    /// \sa SetRenderType()
    GRAPH_API_H
    bool HasRenderType() const;

    /// @}

    // ---------------------------------------------------------------
    /// \name UsdAttribute API
    // ---------------------------------------------------------------

    /// @{

    /// Speculative constructor that will produce a valid GraphInput when
    /// \p attr already represents a shade Input, and produces an \em invalid 
    /// GraphInput otherwise (i.e. the explicit bool conversion operator will 
    /// return false).
    GRAPH_API_H
    explicit GraphInput(const UsdAttribute &attr);

    /// Test whether a given UsdAttribute represents a valid Input, which
    /// implies that creating a GraphInput from the attribute will succeed.
    ///
    /// Success implies that \c attr.IsDefined() is true.
    GRAPH_API_H
    static bool IsInput(const UsdAttribute &attr);

    /// Test if this name has a namespace that indicates it could be an
    /// input.
    GRAPH_API_H
    static bool IsInterfaceInputName(const std::string & name);

    /// Explicit UsdAttribute extractor.
    const UsdAttribute &GetAttr() const { return _attr; }

    /// Allow GraphInput to auto-convert to UsdAttribute, so you can
    /// pass a GraphInput to any function that accepts a UsdAttribute or
    /// const-ref thereto.
    operator const UsdAttribute & () const { return GetAttr(); }

    /// Return true if the wrapped UsdAttribute is defined, and in addition the 
    /// attribute is identified as an input.
    bool IsDefined() const {
        return _attr && IsInput(_attr);
    }

    /// Set documentation string for this Input.
    /// \sa UsdObject::SetDocumentation()
    GRAPH_API_H
    bool SetDocumentation(const std::string& docs) const;

    /// Get documentation string for this Input.
    /// \sa UsdObject::GetDocumentation()
    GRAPH_API_H
    std::string GetDocumentation() const;

    /// Set the displayGroup metadata for this Input,  i.e. hinting for the
    /// location and nesting of the attribute.
    /// \sa UsdProperty::SetDisplayGroup(), UsdProperty::SetNestedDisplayGroup()
    GRAPH_API_H
    bool SetDisplayGroup(const std::string& displayGroup) const;

    /// Get the displayGroup metadata for this Input, i.e. hint for the location 
    /// and nesting of the attribute.
    /// \sa UsdProperty::GetDisplayGroup(), UsdProperty::GetNestedDisplayGroup()
    GRAPH_API_H
    std::string GetDisplayGroup() const;

    /// @}

    /// Return true if this Input is valid for querying and authoring
    /// values and metadata, which is identically equivalent to IsDefined().
    explicit operator bool() const { 
        return IsDefined(); 
    }

    /// Equality comparison. Returns true if \a lhs and \a rhs represent the 
    /// same GraphInput, false otherwise.
    friend bool operator==(const GraphInput &lhs, const GraphInput &rhs) {
        return lhs.GetAttr() == rhs.GetAttr();
    }

    // -------------------------------------------------------------------------
    /// \name Connections API
    // -------------------------------------------------------------------------
    /// @{

    /// Determines whether this Input can be connected to the given 
    /// source attribute, which can be an input or an output.
    /// 
    /// \sa GraphConnectableAPI::CanConnect
    GRAPH_API_H
    bool CanConnect(const UsdAttribute &source) const;

    /// \overload
    GRAPH_API_H
    bool CanConnect(const GraphInput &sourceInput) const;

    /// \overload
    GRAPH_API_H
    bool CanConnect(const GraphOutput &sourceOutput) const;

    /// Authors a connection for this Input to the source described by the 
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
    GRAPH_API_H
    bool ConnectToSource(
        GraphConnectableAPI const &source, 
        TfToken const &sourceName, 
        GraphAttributeType const sourceType=GraphAttributeType::Output,
        SdfValueTypeName typeName=SdfValueTypeName()) const;

    /// Authors a connection for this Input to the source at the given path.
    /// 
    /// \sa GraphConnectableAPI::ConnectToSource
    ///
    GRAPH_API_H
    bool ConnectToSource(SdfPath const &sourcePath) const;

    /// Connects this Input to the given input, \p sourceInput.
    /// 
    /// \sa GraphConnectableAPI::ConnectToSource
    ///
    GRAPH_API_H
    bool ConnectToSource(GraphInput const &sourceInput) const;

    /// Connects this Input to the given output, \p sourceOutput.
    /// 
    /// \sa GraphConnectableAPI::ConnectToSource
    ///
    GRAPH_API_H
    bool ConnectToSource(GraphOutput const &sourceOutput) const;

    /// Finds the source of a connection for this Input.
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
    GRAPH_API_H
    bool GetConnectedSource(GraphConnectableAPI *source, 
                            TfToken *sourceName,
                            GraphAttributeType *sourceType) const;

    /// Returns the "raw" (authored) connected source paths for this Input.
    /// 
    /// \sa GraphConnectableAPI::GetRawConnectedSourcePaths
    ///
    GRAPH_API_H
    bool GetRawConnectedSourcePaths(SdfPathVector *sourcePaths) const;

    /// Returns true if and only if this Input is currently connected to a 
    /// valid (defined) source. 
    ///
    /// \sa GraphConnectableAPI::HasConnectedSource
    /// 
    GRAPH_API_H
    bool HasConnectedSource() const;

    /// Returns true if the connection to this Input's source, as returned by 
    /// GetConnectedSource(), is authored across a specializes arc, which is 
    /// used to denote a base material.
    /// 
    /// \sa GraphConnectableAPI::IsSourceConnectionFromBaseMaterial
    ///
    GRAPH_API_H
    bool IsSourceConnectionFromBaseMaterial() const;

    /// Disconnect source for this Input.
    /// 
    /// \sa GraphConnectableAPI::DisconnectSource
    ///
    GRAPH_API_H
    bool DisconnectSource() const;

    /// Clears source for this shading property in the current UsdEditTarget.
    ///
    /// Most of the time, what you probably want is DisconnectSource()
    /// rather than this function.
    ///
    /// \sa GraphConnectableAPI::ClearSource
    ///
    GRAPH_API_H
    bool ClearSource() const;

    /// @}

    // -------------------------------------------------------------------------
    /// \name Connectability API
    // -------------------------------------------------------------------------
    /// @{
        
    /// \brief Set the connectability of the Input. 
    /// 
    /// In certain shading data models, there is a need to distinguish which 
    /// inputs <b>can</b> vary over a surface from those that must be 
    /// <b>uniform</b>. This is accomplished in Graph by limiting the 
    /// connectability of the input. This is done by setting the 
    /// "connectability" metadata on the associated attribute.
    /// 
    /// Connectability of an Input can be set to GraphTokens->full or 
    /// GraphTokens->interfaceOnly. 
    /// 
    /// \li <b>full</b> implies that  the Input can be connected to any other 
    /// Input or Output.  
    /// \li <b>interfaceOnly</b> implies that the Input can only be connected to 
    /// a NodeGraph Input (which represents an interface override, not a 
    /// render-time dataflow connection), or another Input whose connectability 
    /// is also "interfaceOnly".
    /// 
    /// The default connectability of an input is GraphTokens->full.
    /// 
    /// \sa SetConnectability()
    GRAPH_API_H
    bool SetConnectability(const TfToken &connectability) const;

    /// \brief Returns the connectability of the Input.
    /// 
    /// \sa SetConnectability()
    GRAPH_API_H
    TfToken GetConnectability() const;

    /// \brief Clears any authored connectability on the Input.
    /// 
    GRAPH_API_H
    bool ClearConnectability() const;

    /// @}

    // -------------------------------------------------------------------------
    /// \name Connected Value API
    // -------------------------------------------------------------------------
    /// @{

    /// \brief Find what is connected to an Input recursively
    ///
    /// When tracing connections within networks that contain GraphNodeGraph
    /// nodes, the actual output or value at the end of an input might be
    /// multiple connections removed. The method below resolves this across
    /// multiple physical connections.
    ///
    /// An UsdInput is getting its value from one of these sources:
    /// \li If the input is not connected the UsdAttribute for this input is
    /// returned, but only if it has an authored value. The input attribute
    /// itself carries the value for this input.
    /// \li If the input is connected we follow the connection(s) until we reach
    /// a valid output of a GraphShader node or if we reach a valid
    /// GraphInput attribute of a GraphNodeGraph or GraphMaterial.
    /// Note that we return the last attribute along the connection chain that
    /// has an authored value, which might not be the last attribute in the
    /// chain itself.
    ///
    /// This function returns a valid UsdAttribute if a valid output was
    /// encountered or an input with an authored value. Otherwise an invalid
    /// UsdAttribute is returned. If a valid \p attrType pointer is provided,
    /// the method also returns what type of attribute it found, which is
    /// <b>Invalid</b>, <b>Input</b> or <b>Output</b>.
    GRAPH_API_H
    UsdAttribute GetValueProducingAttribute(
        GraphAttributeType* attrType) const;

    /// @}

private:
    friend class GraphConnectableAPI;

    // Constructor that creates a GraphInput with the given name on the 
    // given prim.
    // \p name here is the unnamespaced name of the input.
    GraphInput(UsdPrim prim,
                  TfToken const &name,
                  SdfValueTypeName const &typeName);
    
    UsdAttribute _attr;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // Graph_INPUT_H
