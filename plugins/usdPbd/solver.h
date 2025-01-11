//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDPBD_GENERATED_SOLVER_H
#define USDPBD_GENERATED_SOLVER_H

/// \file UsdPbd/solver.h

#include "pxr/pxr.h"
#include "pxr/base/vt/value.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usd/collectionAPI.h"

#include "usdPbd/api.h"
#include "usdPbd/tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// PBDSOLVER                                                                  //
// -------------------------------------------------------------------------- //

/// \class UsdPbdSolver
///
/// Defines position based dynamics solver.
///
class UsdPbdSolver : public UsdGeomXform
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaKind
    static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;

    /// Construct a UsdPbdSolver on UsdPrim \p prim .
    /// Equivalent to UsdPbdSolver::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdPbdSolver(const UsdPrim& prim=UsdPrim())
        : UsdGeomXform(prim)
    {
    }

    /// Construct a UsdPbdSolver on the prim held by \p schemaObj .
    /// Should be preferred over UsdPbdSolver(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdPbdSolver(const UsdSchemaBase& schemaObj)
        : UsdGeomXform(schemaObj)
    {
    }

    /// Destructor.
    USDPBD_API
    virtual ~UsdPbdSolver();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDPBD_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdPbdSolver holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdPbdSolver(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDPBD_API
    static UsdPbdSolver
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
    USDPBD_API
    static UsdPbdSolver
    Define(const UsdStagePtr &stage, const SdfPath &path);

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
    // STARTFRAME 
    // --------------------------------------------------------------------- //
    /// Simulation start frame
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `int pbd:startFrame = 1` |
    /// | C++ Type | int |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Int |
    USDPBD_API
    UsdAttribute GetStartFrameAttr() const;

    /// See GetStartFrameAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateStartFrameAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SLEEPTHRESHOLD 
    // --------------------------------------------------------------------- //
    /// Sleep threshold
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float pbd:sleepThreshold = 0.001` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDPBD_API
    UsdAttribute GetSleepThresholdAttr() const;

    /// See GetSleepThresholdAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateSleepThresholdAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SUBSTEPS 
    // --------------------------------------------------------------------- //
    /// Num substeps per frame
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `int pbd:subSteps = 8` |
    /// | C++ Type | int |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Int |
    USDPBD_API
    UsdAttribute GetSubStepsAttr() const;

    /// See GetSubStepsAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateSubStepsAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // GRAVITY 
    // --------------------------------------------------------------------- //
    /// Gravity vector in simulation solver space
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `vector3f pbd:gravity = (0, -9.8, 0)` |
    /// | C++ Type | GfVec3f |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Vector3f |
    USDPBD_API
    UsdAttribute GetGravityAttr() const;

    /// See GetGravityAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateGravityAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SHOWPOINTS 
    // --------------------------------------------------------------------- //
    /// Display particles in viewport
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `bool pbd:showPoints = 1` |
    /// | C++ Type | bool |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
    USDPBD_API
    UsdAttribute GetShowPointsAttr() const;

    /// See GetShowPointsAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateShowPointsAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SHOWCONSTRAINTS 
    // --------------------------------------------------------------------- //
    /// Display constraints in viewport
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `bool pbd:showConstraints = 1` |
    /// | C++ Type | bool |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
    USDPBD_API
    UsdAttribute GetShowConstraintsAttr() const;

    /// See GetShowConstraintsAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPBD_API
    UsdAttribute CreateShowConstraintsAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // BODIES 
    // --------------------------------------------------------------------- //
    /// Simulate Body List
    ///
    USDPBD_API
    UsdRelationship GetBodiesRel() const;

    /// See GetBodiesRel(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create
    USDPBD_API
    UsdRelationship CreateBodiesRel() const;

public:
    // --------------------------------------------------------------------- //
    // COLLIDERS 
    // --------------------------------------------------------------------- //
    /// Collider Object List
    ///
    USDPBD_API
    UsdRelationship GetCollidersRel() const;

    /// See GetCollidersRel(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create
    USDPBD_API
    UsdRelationship CreateCollidersRel() const;

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
