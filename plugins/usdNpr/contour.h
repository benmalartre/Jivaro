//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDNPR_GENERATED_CONTOUR_H
#define USDNPR_GENERATED_CONTOUR_H

/// \file usdNpr/contour.h

#include "pxr/pxr.h"
#include "usdNpr/api.h"
#include "pxr/usd/usdGeom/gprim.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "usdNpr/tokens.h"

#include "pxr/usd/usd/collectionAPI.h" 
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/bboxCache.h" 

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// CONTOUR                                                                    //
// -------------------------------------------------------------------------- //

/// \class UsdNprContour
///
/// Defines a procedurally generated contour for NPR rendering.
///
class UsdNprContour : public UsdGeomGprim
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaKind
    static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;

    /// Construct a UsdNprContour on UsdPrim \p prim .
    /// Equivalent to UsdNprContour::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdNprContour(const UsdPrim& prim=UsdPrim())
        : UsdGeomGprim(prim)
    {
    }

    /// Construct a UsdNprContour on the prim held by \p schemaObj .
    /// Should be preferred over UsdNprContour(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdNprContour(const UsdSchemaBase& schemaObj)
        : UsdGeomGprim(schemaObj)
    {
    }

    /// Destructor.
    USDNPR_API
    virtual ~UsdNprContour();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDNPR_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdNprContour holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdNprContour(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDNPR_API
    static UsdNprContour
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
    USDNPR_API
    static UsdNprContour
    Define(const UsdStagePtr &stage, const SdfPath &path);

protected:
    /// Returns the kind of schema this class belongs to.
    ///
    /// \sa UsdSchemaKind
    USDNPR_API
    UsdSchemaKind _GetSchemaKind() const override;

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    USDNPR_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    USDNPR_API
    const TfType &_GetTfType() const override;

public:
    // --------------------------------------------------------------------- //
    // DRAWSILHOUETTE 
    // --------------------------------------------------------------------- //
    /// 
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `bool drawSilhouette = 1` |
    /// | C++ Type | bool |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
    USDNPR_API
    UsdAttribute GetDrawSilhouetteAttr() const;

    /// See GetDrawSilhouetteAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDNPR_API
    UsdAttribute CreateDrawSilhouetteAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // DRAWBOUNDARY 
    // --------------------------------------------------------------------- //
    /// 
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `bool drawBoundary = 1` |
    /// | C++ Type | bool |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
    USDNPR_API
    UsdAttribute GetDrawBoundaryAttr() const;

    /// See GetDrawBoundaryAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDNPR_API
    UsdAttribute CreateDrawBoundaryAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // DRAWCREASE 
    // --------------------------------------------------------------------- //
    /// 
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `bool drawCrease = 1` |
    /// | C++ Type | bool |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
    USDNPR_API
    UsdAttribute GetDrawCreaseAttr() const;

    /// See GetDrawCreaseAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDNPR_API
    UsdAttribute CreateDrawCreaseAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SILHOUETTEWIDTH 
    // --------------------------------------------------------------------- //
    /// Thickness of the silhouette strokes
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float silhouetteWidth = 1` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDNPR_API
    UsdAttribute GetSilhouetteWidthAttr() const;

    /// See GetSilhouetteWidthAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDNPR_API
    UsdAttribute CreateSilhouetteWidthAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // BOUNDARYWIDTH 
    // --------------------------------------------------------------------- //
    /// Thickness of the boundary strokes
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float boundaryWidth = 1` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDNPR_API
    UsdAttribute GetBoundaryWidthAttr() const;

    /// See GetBoundaryWidthAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDNPR_API
    UsdAttribute CreateBoundaryWidthAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // CREASEWIDTH 
    // --------------------------------------------------------------------- //
    /// Thickness of the crease strokes
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `float creaseWidth = 1` |
    /// | C++ Type | float |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Float |
    USDNPR_API
    UsdAttribute GetCreaseWidthAttr() const;

    /// See GetCreaseWidthAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDNPR_API
    UsdAttribute CreateCreaseWidthAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // CONTOURVIEWPOINT 
    // --------------------------------------------------------------------- //
    /// View Point
    ///
    USDNPR_API
    UsdRelationship GetContourViewPointRel() const;

    /// See GetContourViewPointRel(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create
    USDNPR_API
    UsdRelationship CreateContourViewPointRel() const;

public:
    // --------------------------------------------------------------------- //
    // CONTOURSURFACES 
    // --------------------------------------------------------------------- //
    /// Surfaces from which the strokes are generated
    ///
    USDNPR_API
    UsdRelationship GetContourSurfacesRel() const;

    /// See GetContourSurfacesRel(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create
    USDNPR_API
    UsdRelationship CreateContourSurfacesRel() const;

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
    USDNPR_API
    std::vector<UsdPrim> GetContourSurfaces() const;

    USDNPR_API
    UsdCollectionAPI GetContourSurfacesCollectionAPI() const;

    /// Compute the extent for the contour. 
    ///
    /// \return true on success, false if extents was unable to be calculated.
    /// 
    /// On success, extent will contain the axis-aligned bounding box of the
    /// aggregated surfaces.
    ///
    /// This function is to provide easy authoring of extent for usd authoring
    /// tools, hence it is static and acts outside a specific prim (as in 
    /// attribute based methods).
    USDNPR_API
    static bool ComputeExtent(const UsdTimeCode& timeCode, const UsdPrim& prim,
      VtVec3fArray* extent);

};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
