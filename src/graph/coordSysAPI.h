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
#ifndef GRAPH_GENERATED_COORDSYSAPI_H
#define GRAPH_GENERATED_COORDSYSAPI_H

/// \file Graph/coordSysAPI.h

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/usd/usd/apiSchemaBase.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "./tokens.h"

#include "pxr/usd/usdGeom/xformable.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// COORDSYSAPI                                                                //
// -------------------------------------------------------------------------- //

/// \class GraphCoordSysAPI
///
/// CoordSysAPI provides a way to designate, name,
/// and discover coordinate systems.
/// 
/// Coordinate systems are implicitly established by UsdGeomXformable
/// prims, using their local space.  That coordinate system may be
/// bound (i.e., named) from another prim.  The binding is encoded
/// as a single-target relationship in the "coordSys:" namespace.
/// Coordinate system bindings apply to descendants of the prim
/// where the binding is expressed, but names may be re-bound by
/// descendant prims.
/// 
/// Named coordinate systems are useful in animation workflows.
/// An example is camera base sculpting.  
/// Using the paint coordinate frame avoids the need to assign 
/// a UV set to the object, and can be a concise way to project
/// sculpt across a collection of objects with a single shared
/// paint coordinate system.
/// 
/// This is a non-applied API schema.
/// 
///
class GraphCoordSysAPI : public UsdAPISchemaBase
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaType
    static const UsdSchemaType schemaType = UsdSchemaType::NonAppliedAPI;

    /// Construct a GraphCoordSysAPI on UsdPrim \p prim .
    /// Equivalent to GraphCoordSysAPI::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit GraphCoordSysAPI(const UsdPrim& prim=UsdPrim())
        : UsdAPISchemaBase(prim)
    {
    }

    /// Construct a GraphCoordSysAPI on the prim held by \p schemaObj .
    /// Should be preferred over GraphCoordSysAPI(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit GraphCoordSysAPI(const UsdSchemaBase& schemaObj)
        : UsdAPISchemaBase(schemaObj)
    {
    }

    /// Destructor.
    GRAPH_API
    virtual ~GraphCoordSysAPI();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    GRAPH_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a GraphCoordSysAPI holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// GraphCoordSysAPI(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    GRAPH_API
    static GraphCoordSysAPI
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
