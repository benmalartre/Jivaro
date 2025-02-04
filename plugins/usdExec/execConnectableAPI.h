//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDEXEC_GENERATED_EXECCONNECTABLEAPI_H
#define USDEXEC_GENERATED_EXECCONNECTABLEAPI_H

/// \file usdExec/execConnectableAPI.h

#include "pxr/pxr.h"
#include "pxr/usd/usdExec/api.h"
#include "pxr/usd/usd/apiSchemaBase.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"

#include "pxr/usd/usd/typed.h"
#include "pxr/usd/usdExec/tokens.h"
#include "pxr/usd/usdExec/execInput.h"
#include "pxr/usd/usdExec/execOutput.h"
#include "pxr/usd/usdExec/execTypes.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// EXECCONNECTABLEAPI                                                         //
// -------------------------------------------------------------------------- //

/// \class UsdExecConnectableAPI
///
/// ExecConnectableAPI is an API schema that provides a common
/// interface for creating outputs and making connections between 
/// parameters and outputs. The interface is common to all UsdExec schemas
/// that support Inputs and Outputs, which currently includes UsdExecNode,
/// UsdExecGraph, and UsdExecBundle .
/// 
/// One can construct a ExecConnectableAPI directly from a UsdPrim, or
/// from objects of any of the schema classes listed above.  If it seems
/// onerous to need to construct a secondary schema object to interact with
/// Inputs and Outputs, keep in mind that any function whose purpose is either
/// to walk node networks via their connections, or to create such
/// networks, can typically be written entirely in terms of 
/// ExecConnectableAPI objects, without needing to care what the underlying
/// prim type is.
/// 
/// Additionally, the most common ExecConnectableAPI behaviors
/// (creating Inputs and Outputs, and making connections) are wrapped as
/// convenience methods on the prim schema classes (creation) and 
/// UsdExecInput and UsdExecOutput.
/// 
///
class UsdExecConnectableAPI : public UsdAPISchemaBase
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaKind
    static const UsdSchemaKind schemaKind = UsdSchemaKind::NonAppliedAPI;

    /// Construct a UsdExecConnectableAPI on UsdPrim \p prim .
    /// Equivalent to UsdExecConnectableAPI::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdExecConnectableAPI(const UsdPrim& prim=UsdPrim())
        : UsdAPISchemaBase(prim)
    {
    }

    /// Construct a UsdExecConnectableAPI on the prim held by \p schemaObj .
    /// Should be preferred over UsdExecConnectableAPI(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdExecConnectableAPI(const UsdSchemaBase& schemaObj)
        : UsdAPISchemaBase(schemaObj)
    {
    }

    /// Destructor.
    USDEXEC_API
    virtual ~UsdExecConnectableAPI();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDEXEC_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdExecConnectableAPI holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdExecConnectableAPI(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDEXEC_API
    static UsdExecConnectableAPI
    Get(const UsdStagePtr &stage, const SdfPath &path);


protected:
    /// Returns the kind of schema this class belongs to.
    ///
    /// \sa UsdSchemaKind
    USDEXEC_API
    UsdSchemaKind _GetSchemaKind() const override;

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    USDEXEC_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    USDEXEC_API
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

    public:
    /// Returns true if the prim is a container.
    ///
    /// The underlying prim type may provide runtime behavior
    /// that defines whether it is a container.
    USDEXEC_API
    bool IsContainer() const;

    /// \name Connections 
    /// 
    /// Inputs and outputs on ExecNode and ExecGraph are connectable.
    /// This section provides API for authoring and managing these connections
    /// in a procedural network.
    /// 
    /// @{

    /// Determines whether the given input can be connected to the given 
    /// source attribute, which can be an input or an output.
    /// 
    /// The result depends on the "connectability" of the input and the source 
    /// attributes.  Depending on the prim type, this may require the plugin
    /// that defines connectability behavior for that prim type be loaded.
    /// 
    /// \sa UsdExecInput::SetConnectability
    /// \sa UsdExecInput::GetConnectability
    USDEXEC_API
    static bool CanConnect(const UsdExecInput &input, 
                           const UsdAttribute &source);

    /// \overload
    USDEXEC_API
    static bool CanConnect(const UsdExecInput &input, 
                           const UsdExecInput &sourceInput) {
        return CanConnect(input, sourceInput.GetAttr());
    }

    /// \overload
    USDEXEC_API
    static bool CanConnect(const UsdExecInput &input, 
                           const UsdExecOutput &sourceOutput) {
        return CanConnect(input, sourceOutput.GetAttr());
    }

    /// Determines whether the given output can be connected to the given 
    /// source attribute, which can be an input or an output.
    /// 
    /// 
    /// \p source is an optional argument. If a valid UsdAttribute is supplied
    /// for it, this method will return true only if the source attribute is 
    /// owned by a descendant of the node-graph owning the output.
    ///
    USDEXEC_API
    static bool CanConnect(const UsdExecOutput &output, 
                           const UsdAttribute &source=UsdAttribute());

    /// \overload
    USDEXEC_API
    static bool CanConnect(const UsdExecOutput &output, 
                           const UsdExecInput &sourceInput) {
        return CanConnect(output, sourceInput.GetAttr());
    }

    /// \overload
    USDEXEC_API
    static bool CanConnect(const UsdExecOutput &output, 
                           const UsdExecOutput &sourceOutput) {
        return CanConnect(output, sourceOutput.GetAttr());
    }

    using ConnectionModification = UsdExecConnectionModification;

    /// Authors a connection for a given shading attribute \p attr. 
    /// 
    /// \p attr can represent a parameter, an input or an output.
    /// \p source is a struct that describes the upstream source attribute
    /// with all the information necessary to make a connection. See the
    /// documentation for UsdExecConnectionSourceInfo.
    /// \p mod describes the operation that should be applied to the list of
    /// connections. By default the new connection will replace any existing
    /// connections, but it can add to the list of connections to represent
    /// multiple input connections.
    /// 
    /// \return
    /// \c true if a connection was created successfully.
    /// \c false if \p attr or \p source is invalid.
    /// 
    /// \note This method does not verify the connectability of the shading
    /// attribute to the source. Clients must invoke CanConnect() themselves
    /// to ensure compatibility.
    /// \note The source shading attribute is created if it doesn't exist
    /// already.
    ///
    USDEXEC_API
    static bool ConnectToSource(
        UsdAttribute const &attr,
        UsdExecConnectionSourceInfo const &source,
        ConnectionModification const mod = ConnectionModification::Replace);

    /// \overload
    USDEXEC_API
    static bool ConnectToSource(
        UsdExecInput const &input,
        UsdExecConnectionSourceInfo const &source,
        ConnectionModification const mod = ConnectionModification::Replace)
    {
        return ConnectToSource(input.GetAttr(), source, mod);
    }

    /// \overload
    USDEXEC_API
    static bool ConnectToSource(
        UsdExecOutput const &output,
        UsdExecConnectionSourceInfo const &source,
        ConnectionModification const mod = ConnectionModification::Replace)
    {
        return ConnectToSource(output.GetAttr(), source, mod);
    }

    /// \deprecated Please use the versions that take a
    /// UsdExecConnectionSourceInfo to describe the upstream source
    /// \overload
    USDEXEC_API
    static bool ConnectToSource(
        UsdAttribute const &attr,
        UsdExecConnectableAPI const &source,
        TfToken const &sourceName,
        UsdExecAttributeType const sourceType=UsdExecAttributeType::Output,
        SdfValueTypeName typeName=SdfValueTypeName());

    /// \deprecated
    /// \overload
    USDEXEC_API
    static bool ConnectToSource(
        UsdExecInput const &input,
        UsdExecConnectableAPI const &source,
        TfToken const &sourceName,
        UsdExecAttributeType const sourceType=UsdExecAttributeType::Output,
        SdfValueTypeName typeName=SdfValueTypeName())
    {
        return ConnectToSource(input.GetAttr(), source, sourceName, sourceType,
            typeName);
    }

    /// \deprecated
    /// \overload
    USDEXEC_API
    static bool ConnectToSource(
        UsdExecOutput const &output,
        UsdExecConnectableAPI const &source,
        TfToken const &sourceName,
        UsdExecAttributeType const sourceType=UsdExecAttributeType::Output,
        SdfValueTypeName typeName=SdfValueTypeName())
    {
        return ConnectToSource(output.GetAttr(), source, sourceName, sourceType,
            typeName);
    }

    /// \overload
    /// 
    /// Connect the given shading attribute to the source at path, \p sourcePath.
    /// 
    /// \p sourcePath should be the fully namespaced property path. 
    /// 
    /// This overload is provided for convenience, for use in contexts where 
    /// the prim types are unknown or unavailable.
    /// 
    USDEXEC_API
    static bool ConnectToSource(UsdAttribute const &attr,
                                SdfPath const &sourcePath);

    /// \overload
    USDEXEC_API
    static bool ConnectToSource(UsdExecInput const &input,
                                SdfPath const &sourcePath) {
        return ConnectToSource(input.GetAttr(), sourcePath);
    }

    /// \overload
    USDEXEC_API
    static bool ConnectToSource(UsdExecOutput const &output,
                                SdfPath const &sourcePath) {
        return ConnectToSource(output.GetAttr(), sourcePath);
    }

    /// \overload
    /// 
    /// Connect the given shading attribute to the given source input.
    /// 
    USDEXEC_API
    static bool ConnectToSource(UsdAttribute const &attr,
                                UsdExecInput const &sourceInput);

    /// \overload
    USDEXEC_API
    static bool ConnectToSource(UsdExecInput const &input,
                                UsdExecInput const &sourceInput) {
        return ConnectToSource(input.GetAttr(), sourceInput);
    }

    /// \overload
    USDEXEC_API
    static bool ConnectToSource(UsdExecOutput const &output,
                                UsdExecInput const &sourceInput) {
        return ConnectToSource(output.GetAttr(), sourceInput);
    }

    /// \overload
    /// 
    /// Connect the given shading attribute to the given source output.
    /// 
    USDEXEC_API
    static bool ConnectToSource(UsdAttribute const &attr,
                                UsdExecOutput const &sourceOutput);

    /// \overload
    USDEXEC_API
    static bool ConnectToSource(UsdExecInput const &input,
                                UsdExecOutput const &sourceOutput) {
        return ConnectToSource(input.GetAttr(), sourceOutput);
    }

    /// \overload
    USDEXEC_API
    static bool ConnectToSource(UsdExecOutput const &output,
                                UsdExecOutput const &sourceOutput) {
        return ConnectToSource(output.GetAttr(), sourceOutput);
    }

    /// Authors a list of connections for a given attribute
    /// \p attr.
    /// 
    /// \p attr can represent a parameter, an input or an output.
    /// \p sourceInfos is a vector of structs that describes the upstream source
    /// attributes with all the information necessary to make all the
    /// connections. See the documentation for UsdExecConnectionSourceInfo.
    /// 
    /// \return 
    /// \c true if all connection were created successfully.
    /// \c false if the \p attr or one of the sources are invalid.
    /// 
    /// \note A valid connection is one that has a valid
    /// \p UsdExecConnectionSourceInfo, which requires the existence of the
    /// upstream source prim. It does not require the existence of the source
    /// attribute as it will be create if necessary.
    USDEXEC_API
    static bool SetConnectedSources(
        UsdAttribute const &attr,
        std::vector<UsdExecConnectionSourceInfo> const &sourceInfos);


    /// \deprecated Shading attributes can have multiple connections and so
    /// using GetConnectedSources is needed in general
    /// 
    /// Finds the source of a connection for the given shading attribute.
    /// 
    /// \p attr is the shading attribute whose connection we want to
    /// interrogate.
    /// \p source is an output parameter which will be set to the source 
    /// connectable prim.
    /// \p sourceName will be set to the name of the source shading attribute, 
    /// which may be an input or an output, as specified by \p sourceType
    /// \p sourceType will have the type of the source shading attribute, i.e.
    /// whether it is an \c Input or \c Output
    ///
    /// \return 
    /// \c true if the shading attribute is connected to a valid, defined source
    /// attribute.
    /// \c false if the shading attribute is not connected to a single, defined 
    /// source attribute.
    /// 
    /// \note Previously this method would silently return false for multiple
    /// connections. We are changing the behavior of this method to return the
    /// result for the first connection and issue a TfWarn about it. We want to
    /// encourage clients to use GetConnectedSources going forward.
    /// \note The python wrapping for this method returns a 
    /// (source, sourceName, sourceType) tuple if the parameter is connected, 
    /// else \c None
    USDEXEC_API
    static bool GetConnectedSource(UsdAttribute const &attr,
                                   UsdExecConnectableAPI *source,
                                   TfToken *sourceName,
                                   UsdExecAttributeType *sourceType);

    /// \deprecated
    /// \overload
    USDEXEC_API
    static bool GetConnectedSource(UsdExecInput const &input,
                                   UsdExecConnectableAPI *source,
                                   TfToken *sourceName,
                                   UsdExecAttributeType *sourceType) {
        return GetConnectedSource(input.GetAttr(), source, sourceName,
                                  sourceType);
    }

    /// \deprecated
    /// \overload
    USDEXEC_API
    static bool GetConnectedSource(UsdExecOutput const &output,
                                   UsdExecConnectableAPI *source,
                                   TfToken *sourceName,
                                   UsdExecAttributeType *sourceType) {
        return GetConnectedSource(output.GetAttr(), source, sourceName,
                                  sourceType);
    }

    /// Finds the valid sources of connections for the given shading attribute.
    /// 
    /// \p attr is the shading attribute whose connections we want to
    /// interrogate.
    /// \p invalidSourcePaths is an optional output parameter to collect the
    /// invalid source paths that have not been reported in the returned vector.
    /// 
    /// Returns a vector of \p UsdExecConnectionSourceInfo structs with
    /// information about each upsteam attribute. If the vector is empty, there
    /// have been no connections.
    /// 
    /// \note A valid connection requires the existence of the source attribute
    /// and also requires that the source prim is UsdExecConnectableAPI
    /// compatible.
    /// \note The python wrapping returns a tuple with the valid connections
    /// first, followed by the invalid source paths.
    USDEXEC_API
    static UsdExecSourceInfoVector GetConnectedSources(
        UsdAttribute const &attr,
        SdfPathVector *invalidSourcePaths = nullptr);

    /// \overload
    USDEXEC_API
    static UsdExecSourceInfoVector GetConnectedSources(
        UsdExecInput const &input,
        SdfPathVector *invalidSourcePaths = nullptr);

    /// \overload
    USDEXEC_API
    static UsdExecSourceInfoVector GetConnectedSources(
        UsdExecOutput const &output,
        SdfPathVector *invalidSourcePaths = nullptr);

    /// \deprecated Please us GetConnectedSources to retrieve multiple
    /// connections
    ///
    /// Returns the "raw" (authored) connected source paths for the given 
    /// shading attribute.
    USDEXEC_API
    static bool GetRawConnectedSourcePaths(UsdAttribute const &attr, 
                                           SdfPathVector *sourcePaths);

    /// \deprecated
    /// \overload
    USDEXEC_API
    static bool GetRawConnectedSourcePaths(UsdExecInput const &input, 
                                           SdfPathVector *sourcePaths) {
        return GetRawConnectedSourcePaths(input.GetAttr(), sourcePaths);
    }

    /// \deprecated
    /// \overload
    USDEXEC_API
    static bool GetRawConnectedSourcePaths(UsdExecOutput const &output, 
                                           SdfPathVector *sourcePaths) {
        return GetRawConnectedSourcePaths(output.GetAttr(), sourcePaths);
    }

    /// Returns true if and only if the shading attribute is currently connected
    /// to at least one valid (defined) source.
    ///
    /// If you will be calling GetConnectedSources() afterwards anyways,
    /// it will be \em much faster to instead check if the returned vector is
    /// empty:
    /// \code
    /// UsdExecSourceInfoVector connections =
    ///     UsdExecConnectableAPI::GetConnectedSources(attribute);
    /// if (!connections.empty()){
    ///      // process connected attribute
    /// } else {
    ///      // process unconnected attribute
    /// }
    /// \endcode
    USDEXEC_API
    static bool HasConnectedSource(const UsdAttribute &attr);

    /// \overload
    USDEXEC_API
    static bool HasConnectedSource(const UsdExecInput &input) {
        return HasConnectedSource(input.GetAttr());
    }

    /// \overload
    USDEXEC_API
    static bool HasConnectedSource(const UsdExecOutput &output) {
        return HasConnectedSource(output.GetAttr());
    }

    /// Disconnect source for this shading attribute.
    ///
    /// If \p sourceAttr is valid it will disconnect the connection to this
    /// upstream attribute. Otherwise it will disconnect all connections by
    /// authoring an empty list of connections for the attribute \p attr.
    ///
    /// This may author more scene description than you might expect - we define
    /// the behavior of disconnect to be that, even if a shading attribute 
    /// becomes connected in a weaker layer than the current UsdEditTarget, the
    /// attribute will \em still be disconnected in the composition, therefore
    /// we must "block" it in the current UsdEditTarget. 
    ///
    /// \sa ConnectToSource().
    USDEXEC_API
    static bool DisconnectSource(
        UsdAttribute const &attr,
        UsdAttribute const &sourceAttr = UsdAttribute());

    /// \overload
    USDEXEC_API
    static bool DisconnectSource(
        UsdExecInput const &input,
        UsdAttribute const &sourceAttr = UsdAttribute()) {
        return DisconnectSource(input.GetAttr(), sourceAttr);
    }

    /// \overload
    USDEXEC_API
    static bool DisconnectSource(
        UsdExecOutput const &output,
        UsdAttribute const &sourceAttr = UsdAttribute()) {
        return DisconnectSource(output.GetAttr(), sourceAttr);
    }

    /// Clears sources for this shading attribute in the current UsdEditTarget.
    ///
    /// Most of the time, what you probably want is DisconnectSource()
    /// rather than this function.
    ///
    /// \sa DisconnectSource()
    USDEXEC_API
    static bool ClearSources(UsdAttribute const &attr);

    /// \overload
    USDEXEC_API
    static bool ClearSources(UsdExecInput const &input) {
        return ClearSources(input.GetAttr());
    }

    /// \overload
    USDEXEC_API
    static bool ClearSources(UsdExecOutput const &output) {
        return ClearSources(output.GetAttr());
    }

    /// \deprecated This is the older version that only referenced a single
    /// source. Please use ClearSources instead.
    USDEXEC_API
    static bool ClearSource(UsdAttribute const &attr) {
        return ClearSources(attr);
    }

    /// \deprecated
    /// \overload
    USDEXEC_API
    static bool ClearSource(UsdExecInput const &input) {
        return ClearSources(input.GetAttr());
    }

    /// \deprecated
    /// \overload
    USDEXEC_API
    static bool ClearSource(UsdExecOutput const &output) {
        return ClearSources(output.GetAttr());
    }

    /// Return true if the \p schemaType has a valid execConnectableAPIBehavior
    /// registered, false otherwise.
    /// To check if a prim's execConnectableAPI has a behavior defined, use
    /// UsdSchemaBase::operator bool().
    USDEXEC_API
    static bool HasConnectableAPI(const TfType& schemaType);

    /// Return true if the schema type \p T has a execConnectableAPIBehavior
    /// registered, false otherwise.
    template <typename T>
    static bool HasConnectableAPI()
    {
        static_assert(std::is_base_of<UsdTyped, T>::value, 
                "Provided type must derive UsdTyped.");
        return HasConnectableAPI(TfType::Find<T>());
    };

    /// @}


    /// \name Outputs 
    /// @{

    /// Create an output, which represents and externally computed, typed value.
    /// Outputs on node-graphs can be connected. 
    /// 
    /// The attribute representing an output is created in the "outputs:" 
    /// namespace.
    /// 
    USDEXEC_API
    UsdExecOutput CreateOutput(const TfToken& name,
                                const SdfValueTypeName& typeName) const;

    /// Return the requested output if it exists.
    /// 
    /// \p name is the unnamespaced base name.
    ///
    USDEXEC_API
    UsdExecOutput GetOutput(const TfToken &name) const;

    /// Returns all outputs on the connectable prim (i.e. node or node-graph). 
    /// Outputs are represented by attributes in the "outputs:" namespace.
    /// If \p onlyAuthored is true (the default), then only return authored
    /// attributes; otherwise, this also returns un-authored builtins.
    ///
    USDEXEC_API
    std::vector<UsdExecOutput> GetOutputs(
          bool onlyAuthored=true) const;

    /// @}

    /// \name Inputs 
    /// @{
        
    /// Create an input which can both have a value and be connected.
    /// The attribute representing the input is created in the "inputs:" 
    /// namespace.
    /// 
    USDEXEC_API
    UsdExecInput CreateInput(const TfToken& name,
                               const SdfValueTypeName& typeName) const;

    /// Return the requested input if it exists.
    /// 
    /// \p name is the unnamespaced base name.
    /// 
    USDEXEC_API
    UsdExecInput GetInput(const TfToken &name) const;

    /// Returns all inputs on the connectable prim (i.e. node or node-graph). 
    /// Inputs are represented by attributes in the "inputs:" namespace.
    /// If \p onlyAuthored is true (the default), then only return authored
    /// attributes; otherwise, this also returns un-authored builtins.
    ///
    USDEXEC_API
    std::vector<UsdExecInput> GetInputs(
          bool onlyAuthored=true) const;

    /// @}
};

/// A compact struct to represent a bundle of information about an upstream
/// source attribute
struct UsdExecConnectionSourceInfo {
    /// \p source is the connectable prim that produces or contains a value
    /// for the given shading attribute.
    UsdExecConnectableAPI source;
    /// \p sourceName is the name of the shading attribute that is the target
    /// of the connection. This excludes any namespace prefix that determines
    /// the type of the source (eg, output).
    TfToken sourceName;
    /// \p sourceType is used to indicate the type of the shading attribute
    /// that is the target of the connection. The source type is used to
    /// determine the namespace prefix that must be attached to \p sourceName
    /// to determine the source full attribute name.
    UsdExecAttributeType sourceType = UsdExecAttributeType::Invalid;
    /// \p typeName, if specified, is the typename of the attribute to create
    /// on the source if it doesn't exist when creating a connection
    SdfValueTypeName typeName;

    UsdExecConnectionSourceInfo() = default;
    explicit UsdExecConnectionSourceInfo(
        UsdExecConnectableAPI const &source_,
        TfToken const &sourceName_,
        UsdExecAttributeType sourceType_,
        SdfValueTypeName typeName_ = SdfValueTypeName())
    : source(source_)
    , sourceName(sourceName_)
    , sourceType(sourceType_)
    , typeName(typeName_)
    {}
    explicit UsdExecConnectionSourceInfo(UsdExecInput const &input)
        : source(input.GetPrim())
        , sourceName(input.GetBaseName())
        , sourceType(UsdExecAttributeType::Input)
        , typeName(input.GetAttr().GetTypeName())
    {}
    explicit UsdExecConnectionSourceInfo(UsdExecOutput const &output)
        : source(output.GetPrim())
        , sourceName(output.GetBaseName())
        , sourceType(UsdExecAttributeType::Output)
        , typeName(output.GetAttr().GetTypeName())
    {}
    /// Construct the information for this struct from a property path. The
    /// source attribute does not have to exist, but the \p sourcePath needs to
    /// have a valid prefix to identify the sourceType. The source prim needs
    /// to exist and be UsdExecConnectableAPI compatible
    USDEXEC_API
    explicit UsdExecConnectionSourceInfo(
        UsdStagePtr const& stage,
        SdfPath const& sourcePath);

    /// Return true if this source info is valid for setting up a connection
    bool IsValid() const {
        // typeName can be invalid, so we don't check it. Order of checks is in
        // order of cost (cheap to expensive).
        // Note, for the source we only check that the prim is valid. We do not
        // verify that the prim is compatibel with UsdExecConnectableAPI. This
        // makes it possible to target pure overs
        return (sourceType != UsdExecAttributeType::Invalid) &&
               !sourceName.IsEmpty() &&
               (bool)source.GetPrim();
    }
    explicit operator bool() const {
        return IsValid();
    }
    bool operator==(UsdExecConnectionSourceInfo const& other) const {
        // We don't compare the typeName, since it is optional
        return sourceName == other.sourceName &&
               sourceType == other.sourceType &&
               source.GetPrim() == other.source.GetPrim();
    }
    bool operator!=(const UsdExecConnectionSourceInfo &other) const {
        return !(*this == other);
    }
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
