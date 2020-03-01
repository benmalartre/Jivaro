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
#include "Graph/api.h"
#include "pxr/usd/usd/apiSchemaBase.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"

#include "node.h"
#include "nodeGraph.h"


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
/// interface for creating outputs and making connections between nodes 
/// parameters and outputs. The interface is common to all Graph schemas
/// that support Inputs and Outputs, which currently includes Node,
/// NodeGraph, and NodeComposer .
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
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
