//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "usdNpr/tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

UsdNprTokensType::UsdNprTokensType() :
    boundaries("boundaries", TfToken::Immortal),
    boundaryWidth("boundaryWidth", TfToken::Immortal),
    contourSurfaces("contour:surfaces", TfToken::Immortal),
    contourViewPoint("contour:viewPoint", TfToken::Immortal),
    creases("creases", TfToken::Immortal),
    creaseWidth("creaseWidth", TfToken::Immortal),
    drawBoundary("drawBoundary", TfToken::Immortal),
    drawCrease("drawCrease", TfToken::Immortal),
    drawSilhouette("drawSilhouette", TfToken::Immortal),
    silhouettes("silhouettes", TfToken::Immortal),
    silhouetteWidth("silhouetteWidth", TfToken::Immortal),
    surfaces("surfaces", TfToken::Immortal),
    Contour("Contour", TfToken::Immortal),
    allTokens({
        boundaries,
        boundaryWidth,
        contourSurfaces,
        contourViewPoint,
        creases,
        creaseWidth,
        drawBoundary,
        drawCrease,
        drawSilhouette,
        silhouettes,
        silhouetteWidth,
        surfaces,
        Contour
    })
{
}

TfStaticData<UsdNprTokensType> UsdNprTokens;

PXR_NAMESPACE_CLOSE_SCOPE
