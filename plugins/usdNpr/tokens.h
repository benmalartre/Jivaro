//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDNPR_TOKENS_H
#define USDNPR_TOKENS_H

/// \file usdNpr/tokens.h

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// This is an automatically generated file (by usdGenSchema.py).
// Do not hand-edit!
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "pxr/pxr.h"
#include "usdNpr/api.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/tf/token.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE


/// \class UsdNprTokensType
///
/// \link UsdNprTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// These tokens are auto-generated from the module's schema, representing
/// property names, for when you need to fetch an attribute or relationship
/// directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
/// manner, and allow the compiler to verify that you spelled the name
/// correctly.
///
/// UsdNprTokens also contains all of the \em allowedTokens values
/// declared for schema builtin attributes of 'token' scene description type.
/// Use UsdNprTokens like so:
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(UsdNprTokens->boundaries);
/// \endcode
struct UsdNprTokensType {
    USDNPR_API UsdNprTokensType();
    /// \brief "boundaries"
    /// 
    /// Boolean - Does the stroke draw boundaries of the surfaces.
    const TfToken boundaries;
    /// \brief "boundaryWidth"
    /// 
    /// UsdNprContour
    const TfToken boundaryWidth;
    /// \brief "contour:surfaces"
    /// 
    /// UsdNprContour
    const TfToken contourSurfaces;
    /// \brief "contour:viewPoint"
    /// 
    /// UsdNprContour
    const TfToken contourViewPoint;
    /// \brief "creases"
    /// 
    /// Boolean - Does the stroke draw creases of the surfaces.
    const TfToken creases;
    /// \brief "creaseWidth"
    /// 
    /// UsdNprContour
    const TfToken creaseWidth;
    /// \brief "drawBoundary"
    /// 
    /// UsdNprContour
    const TfToken drawBoundary;
    /// \brief "drawCrease"
    /// 
    /// UsdNprContour
    const TfToken drawCrease;
    /// \brief "drawSilhouette"
    /// 
    /// UsdNprContour
    const TfToken drawSilhouette;
    /// \brief "silhouettes"
    /// 
    /// Boolean - Does the stroke draw silhouette of the surfaces.
    const TfToken silhouettes;
    /// \brief "silhouetteWidth"
    /// 
    /// UsdNprContour
    const TfToken silhouetteWidth;
    /// \brief "surfaces"
    /// 
    /// Represent surface-linking of a UsdContour prim.
    const TfToken surfaces;
    /// \brief "Contour"
    /// 
    /// Schema identifer and family for UsdNprContour
    const TfToken Contour;
    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var UsdNprTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa UsdNprTokensType
extern USDNPR_API TfStaticData<UsdNprTokensType> UsdNprTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif
