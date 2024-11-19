//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDPBD_GENERATED_BODYAPI_H
#define USDPBD_GENERATED_BODYAPI_H

/// \file UsdPbd/bodyAPI.h

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
// PBDBODYAPI                                                                 //
// -------------------------------------------------------------------------- //

/// \class UsdPbdBodyAPI
///
/// Applies soft body attributes to a deformable prim.
///
class UsdPbdBodyAPI : public UsdAPISchemaBase
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaKind
    static const UsdSchemaKind schemaKind = UsdSchemaKind::SingleApplyAPI;

    /// Construct a UsdPbdBodyAPI on UsdPrim \p prim .
    /// Equivalent to UsdPbdBodyAPI::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdPbdBodyAPI(const UsdPrim& prim=UsdPrim())
        : UsdAPISchemaBase(prim)
    {
    }

    /// Construct a UsdPbdBodyAPI on the prim held by \p schemaObj .
    /// Should be preferred over UsdPbdBodyAPI(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdPbdBodyAPI(const UsdSchemaBase& schemaObj)
        : UsdAPISchemaBase(schemaObj)
    {
    }

    /// Destructor.
    USDPBD_API
    virtual ~UsdPbdBodyAPI();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDPBD_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdPbdBodyAPI holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdPbdBodyAPI(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDPBD_API
    static UsdPbdBodyAPI
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
    /// This information is stored by adding "PbdBodyAPI" to the 
    /// token-valued, listOp metadata \em apiSchemas on the prim.
    /// 
    /// \return A valid UsdPbdBodyAPI object is returned upon success. 
    /// An invalid (or empty) UsdPbdBodyAPI object is returned upon 
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
    static UsdPbdBodyAPI 
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
    // SIMULATIONENABLED 
    // --------------------------------------------------------------------- //
    /// Determines if the PbdBodyAPI is enabled.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `bool pbd:simulationEnabled = 1` |
    /// | C++ Type | bool |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
    USDPBD_API
    UsdAttribute GetSimulationEnabledAttr() const;

    /// See GetSimulationEnabledAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateSimulationEnabledAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // RADIUS 
    // --------------------------------------------------------------------- //
    /// particle radius used by collision detection.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:radius = 0.1` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetRadiusAttr() const;

    /// See GetRadiusAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateRadiusAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SELFCOLLISIONENABLED 
    // --------------------------------------------------------------------- //
    /// self collision detection active state.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `bool pbd:selfCollisionEnabled = 1` |
    /// | C++ Type | bool |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
    USDPBD_API
    UsdAttribute GetSelfCollisionEnabledAttr() const;

    /// See GetSelfCollisionEnabledAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateSelfCollisionEnabledAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SELFCOLLISIONRADIUS 
    // --------------------------------------------------------------------- //
    /// particle radius used by self collision detection.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:selfCollisionRadius = 1` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetSelfCollisionRadiusAttr() const;

    /// See GetSelfCollisionRadiusAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateSelfCollisionRadiusAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SELFCOLLISIONDAMP 
    // --------------------------------------------------------------------- //
    /// self collision damp coefficient.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:selfCollisionDamp = 0.1` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetSelfCollisionDampAttr() const;

    /// See GetSelfCollisionDampAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateSelfCollisionDampAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SELFCOLLISIONFRICTION 
    // --------------------------------------------------------------------- //
    /// self collision friction coefficient.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:selfCollisionFriction = 0.2` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetSelfCollisionFrictionAttr() const;

    /// See GetSelfCollisionFrictionAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateSelfCollisionFrictionAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SELFCOLLISIONRESTITUTION 
    // --------------------------------------------------------------------- //
    /// Self COllision Restitution coefficient.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:selfCollisionRestitution = 0` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetSelfCollisionRestitutionAttr() const;

    /// See GetSelfCollisionRestitutionAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateSelfCollisionRestitutionAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SELFCOLLISIONMAXSEPARATIONVELOCITY 
    // --------------------------------------------------------------------- //
    /// Self Collision Maximum separation velocity.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:selfCollisionMaxSeparationVelocity = 5` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetSelfCollisionMaxSeparationVelocityAttr() const;

    /// See GetSelfCollisionMaxSeparationVelocityAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateSelfCollisionMaxSeparationVelocityAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // MASS 
    // --------------------------------------------------------------------- //
    /// friction coefficient.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:mass = 1` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetMassAttr() const;

    /// See GetMassAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateMassAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // DAMP 
    // --------------------------------------------------------------------- //
    /// damp coefficient.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:damp = 0.1` |
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
    // VELOCITY 
    // --------------------------------------------------------------------- //
    /// Velocity vector in simulation solver space
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `vector3f pbd:velocity = (0, 0, 0)` |
    /// | C++ Type | GfVec3f |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Vector3f |
    USDPBD_API
    UsdAttribute GetVelocityAttr() const;

    /// See GetVelocityAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateVelocityAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

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
