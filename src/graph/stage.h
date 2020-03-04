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
#ifndef GRAPH_GENERATED_STAGE_H
#define GRAPH_GENERATED_STAGE_H

/// \file Graph/stage.h

#include "pxr/pxr.h"
#include "./api.h"
#include "./graph.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "./tokens.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// STAGE                                                                      //
// -------------------------------------------------------------------------- //

/// \class GraphStage
///
/// Class for the terminal stage node
///
/// For any described attribute \em Fallback \em Value or \em Allowed \em Values below
/// that are text/tokens, the actual token is published and defined in \ref GraphTokens.
/// So to set an attribute to the value "rightHanded", use GraphTokens->rightHanded
/// as the value.
///
class GraphStage : public GraphGraph
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaType
    static const UsdSchemaType schemaType = UsdSchemaType::ConcreteTyped;

    /// Construct a GraphStage on UsdPrim \p prim .
    /// Equivalent to GraphStage::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit GraphStage(const UsdPrim& prim=UsdPrim())
        : GraphGraph(prim)
    {
    }

    /// Construct a GraphStage on the prim held by \p schemaObj .
    /// Should be preferred over GraphStage(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit GraphStage(const UsdSchemaBase& schemaObj)
        : GraphGraph(schemaObj)
    {
    }

    /// Destructor.
    GRAPH_API
    virtual ~GraphStage();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    GRAPH_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a GraphStage holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// GraphStage(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    GRAPH_API
    static GraphStage
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
    static GraphStage
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
    // LIFETIMEMANAGEMENT 
    // --------------------------------------------------------------------- //
    /// Encodes the lifetime management of the stage, on-disk or
    /// in-memory
    ///
    /// \n  C++ Type: TfToken
    /// \n  Usd Type: SdfValueTypeNames->Token
    /// \n  Variability: SdfVariabilityUniform
    /// \n  Fallback Value: No Fallback
    /// \n  \ref GraphTokens "Allowed Values": [on-disk, in-memory]
    GRAPH_API
    UsdAttribute GetLifetimeManagementAttr() const;

    /// See GetLifetimeManagementAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    GRAPH_API
    UsdAttribute CreateLifetimeManagementAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // FILENAME 
    // --------------------------------------------------------------------- //
    /// ar-solvable file path on disk
    ///
    /// \n  C++ Type: std::string
    /// \n  Usd Type: SdfValueTypeNames->String
    /// \n  Variability: SdfVariabilityUniform
    /// \n  Fallback Value: No Fallback
    GRAPH_API
    UsdAttribute GetFileNameAttr() const;

    /// See GetFileNameAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    GRAPH_API
    UsdAttribute CreateFileNameAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // LOADPRIMSPATH 
    // --------------------------------------------------------------------- //
    /// Full Path to prims on the stage.
    /// These prims will be associated with state below on loading the stage.
    /// 
    ///
    /// \n  C++ Type: VtArray<std::string>
    /// \n  Usd Type: SdfValueTypeNames->StringArray
    /// \n  Variability: SdfVariabilityUniform
    /// \n  Fallback Value: No Fallback
    GRAPH_API
    UsdAttribute GetLoadPrimsPathAttr() const;

    /// See GetLoadPrimsPathAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    GRAPH_API
    UsdAttribute CreateLoadPrimsPathAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // LOADPRIMSSTATES 
    // --------------------------------------------------------------------- //
    /// Load rule for the associated prim.
    /// - AllRule : Include payloads on the specified prim and all 
    /// descendants.
    /// - OnlyRule : Include payloads on the specified prim but no 
    /// descendants.
    /// - NoneRule : Exclude payloads on the specified prim and all 
    /// descendants.
    /// 
    ///
    /// \n  C++ Type: VtArray<TfToken>
    /// \n  Usd Type: SdfValueTypeNames->TokenArray
    /// \n  Variability: SdfVariabilityUniform
    /// \n  Fallback Value: No Fallback
    /// \n  \ref GraphTokens "Allowed Values": [AllRule, OnlyRule, NoneRule]
    GRAPH_API
    UsdAttribute GetLoadPrimsStatesAttr() const;

    /// See GetLoadPrimsStatesAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    GRAPH_API
    UsdAttribute CreateLoadPrimsStatesAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // POPULATIONMASK 
    // --------------------------------------------------------------------- //
    /// Set of fullpath to prims on the stage.
    /// This set represents a mask that may be applied to a stage to limit 
    /// the prims it populates.
    /// 
    ///
    /// \n  C++ Type: VtArray<std::string>
    /// \n  Usd Type: SdfValueTypeNames->StringArray
    /// \n  Variability: SdfVariabilityUniform
    /// \n  Fallback Value: No Fallback
    GRAPH_API
    UsdAttribute GetPopulationMaskAttr() const;

    /// See GetPopulationMaskAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    GRAPH_API
    UsdAttribute CreatePopulationMaskAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // INPUTSLAYERS 
    // --------------------------------------------------------------------- //
    /// Relationships to the layers composing this stage.
    ///
    /// \n  C++ Type: VtArray<TfToken>
    /// \n  Usd Type: SdfValueTypeNames->TokenArray
    /// \n  Variability: SdfVariabilityVarying
    /// \n  Fallback Value: No Fallback
    GRAPH_API
    UsdAttribute GetInputsLayersAttr() const;

    /// See GetInputsLayersAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    GRAPH_API
    UsdAttribute CreateInputsLayersAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

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
