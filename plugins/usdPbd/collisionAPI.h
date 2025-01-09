//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDPBD_GENERATED_COLLISIONAPI_H
#define USDPBD_GENERATED_COLLISIONAPI_H

/// \file UsdPbd/collisionAPI.h

#include "pxr/pxr.h"
#include "usdPbd/api.h"
#include "pxr/usd/usd/apiSchemaBase.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "usdPbd/tokens.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// PBDCOLLISIONAPI                                                            //
// -------------------------------------------------------------------------- //

/// \class UsdPbdCollisionAPI
///
/// Applies position based dynamics collision attributes to a UsdGeomXformable prim.
///
class UsdPbdCollisionAPI : public UsdAPISchemaBase
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaKind
    static const UsdSchemaKind schemaKind = UsdSchemaKind::SingleApplyAPI;

    /// Construct a UsdPbdCollisionAPI on UsdPrim \p prim .
    /// Equivalent to UsdPbdCollisionAPI::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdPbdCollisionAPI(const UsdPrim& prim=UsdPrim())
        : UsdAPISchemaBase(prim)
    {
    }

    /// Construct a UsdPbdCollisionAPI on the prim held by \p schemaObj .
    /// Should be preferred over UsdPbdCollisionAPI(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdPbdCollisionAPI(const UsdSchemaBase& schemaObj)
        : UsdAPISchemaBase(schemaObj)
    {
    }

    /// Destructor.
    USDPBD_API
    virtual ~UsdPbdCollisionAPI();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDPBD_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdPbdCollisionAPI holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdPbdCollisionAPI(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDPBD_API
    static UsdPbdCollisionAPI
    Get(const UsdStagePtr &stage, const SdfPath &path);


    /// Returns true if this <b>single-apply</b> API schema can be applied to 
    /// the given \p prim. If this schema can not be a applied to the prim, 
    /// this returns false and, if provided, populates \p whyNot with the 
    /// reason it can not be applied.
    /// 
    /// Note that if CanApply returns false, that does not necessarily imply
    /// that calling Apply will fail. Callers are expected to call CanApply
    /// before calling Apply if they want to ensure that it is valid to 
    /// apply a schema.
    /// 
    /// \sa UsdPrim::GetAppliedSchemas()
    /// \sa UsdPrim::HasAPI()
    /// \sa UsdPrim::CanApplyAPI()
    /// \sa UsdPrim::ApplyAPI()
    /// \sa UsdPrim::RemoveAPI()
    ///
    USDPBD_API
    static bool 
    CanApply(const UsdPrim &prim, std::string *whyNot=nullptr);

    /// Applies this <b>single-apply</b> API schema to the given \p prim.
    /// This information is stored by adding "PbdCollisionAPI" to the 
    /// token-valued, listOp metadata \em apiSchemas on the prim.
    /// 
    /// \return A valid UsdPbdCollisionAPI object is returned upon success. 
    /// An invalid (or empty) UsdPbdCollisionAPI object is returned upon 
    /// failure. See \ref UsdPrim::ApplyAPI() for conditions 
    /// resulting in failure. 
    /// 
    /// \sa UsdPrim::GetAppliedSchemas()
    /// \sa UsdPrim::HasAPI()
    /// \sa UsdPrim::CanApplyAPI()
    /// \sa UsdPrim::ApplyAPI()
    /// \sa UsdPrim::RemoveAPI()
    ///
    USDPBD_API
    static UsdPbdCollisionAPI 
    Apply(const UsdPrim &prim);

protected:
    /// Returns the kind of schema this class belongs to.
    ///
    /// \sa UsdSchemaKind
    USDPBD_API
    UsdSchemaKind _GetSchemaKind() const override;

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    USDPBD_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    USDPBD_API
    const TfType &_GetTfType() const override;

public:
    // --------------------------------------------------------------------- //
    // COLLISIONENABLED 
    // --------------------------------------------------------------------- //
    /// Determines if the PbdCollisionAPI is enabled.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `bool pbd:collisionEnabled = 1` |
    /// | C++ Type | bool |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
    USDPBD_API
    UsdAttribute GetCollisionEnabledAttr() const;

    /// See GetCollisionEnabledAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateCollisionEnabledAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // MARGIN 
    // --------------------------------------------------------------------- //
    /// extend search radius by margin for safer collision detection.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:margin = 0.05` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetMarginAttr() const;

    /// See GetMarginAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateMarginAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // DAMP 
    // --------------------------------------------------------------------- //
    /// collision constraint damp.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:collisionDamp = 0.25` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetDampAttr() const;

    /// See GetDampAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateDampAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // COLLISIONSTIFFNESS 
    // --------------------------------------------------------------------- //
    /// collision constraint stiffness.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:collisionStiffness = 0` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetCollisionStiffnessAttr() const;

    /// See GetCollisionStiffnessAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateCollisionStiffnessAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // FRICTION 
    // --------------------------------------------------------------------- //
    /// friction coefficient.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:friction = 0.2` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetFrictionAttr() const;

    /// See GetFrictionAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateFrictionAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // RESTITUTION 
    // --------------------------------------------------------------------- //
    /// Restitution coefficient.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:restitution = 0` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetRestitutionAttr() const;

    /// See GetRestitutionAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateRestitutionAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // MAXSEPARATIONVELOCITY 
    // --------------------------------------------------------------------- //
    /// Maximum separation velocity.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:maxSeparationVelocity = 5` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetMaxSeparationVelocityAttr() const;

    /// See GetMaxSeparationVelocityAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateMaxSeparationVelocityAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

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
