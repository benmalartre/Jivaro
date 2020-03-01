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
#ifndef GRAPH_GENERATED_NODECOMPOSER_H
#define GRAPH_GENERATED_NODECOMPOSER_H

/// \file Graph/nodeComposer.h

#include "pxr/pxr.h"
#include "Graph/api.h"
#include "Graph/nodeGraph.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "Graph/tokens.h"

#include "asset.h"
#include "node.h"
#include "connectableAPI.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// NODECOMPOSER                                                               //
// -------------------------------------------------------------------------- //

/// \class GraphNodeComposer
///
/// A NodeComposer provides a container into which multiple "assets"
/// can add data that defines datas to upstream the pipeline.  Typically
/// this consists of one or more UsdRelationship properties that target
/// other prims of type \em Asset - though a target/client is free to add
/// any data that is suitable.  We <b>strongly advise</b> that all targets
/// adopt the convention that all properties be prefixed with a namespace
/// that identifies the target, e.g. "rel Bob:geom = </Geometry/Head>".
/// 
///
/// For any described attribute \em Fallback \em Value or \em Allowed \em Values below
/// that are text/tokens, the actual token is published and defined in \ref GraphTokens.
/// So to set an attribute to the value "rightHanded", use GraphTokens->rightHanded
/// as the value.
///
class GraphNodeComposer : public GraphNodeGraph
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaType
    static const UsdSchemaType schemaType = UsdSchemaType::ConcreteTyped;

    /// Construct a GraphNodeComposer on UsdPrim \p prim .
    /// Equivalent to GraphNodeComposer::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit GraphNodeComposer(const UsdPrim& prim=UsdPrim())
        : GraphNodeGraph(prim)
    {
    }

    /// Construct a GraphNodeComposer on the prim held by \p schemaObj .
    /// Should be preferred over GraphNodeComposer(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit GraphNodeComposer(const UsdSchemaBase& schemaObj)
        : GraphNodeGraph(schemaObj)
    {
    }

    /// Destructor.
    GRAPH_API
    virtual ~GraphNodeComposer();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    GRAPH_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a GraphNodeComposer holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// GraphNodeComposer(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    GRAPH_API
    static GraphNodeComposer
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
    static GraphNodeComposer
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
    // --------------------------------------------------------------------- //
    // DEFORMED 
    // --------------------------------------------------------------------- //
    /// Describes the <i>deformed geometry</i> output terminal on a NodeComposer.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `token outputs:deformed` |
    /// | C++ Type | TfToken |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
    GRAPH_API
    UsdAttribute GetDeformedAttr() const;

    /// See GetDeformedAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    GRAPH_API
    UsdAttribute CreateDeformedAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

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
