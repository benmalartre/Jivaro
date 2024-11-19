//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// GENERATED FILE.  DO NOT EDIT.
#include "pxr/external/boost/python/class.hpp"
#include "usdNpr/tokens.h"

PXR_NAMESPACE_USING_DIRECTIVE

#define _ADD_TOKEN(cls, name) \
    cls.add_static_property(#name, +[]() { return UsdNprTokens->name.GetString(); });

void wrapUsdNprTokens()
{
    pxr_boost::python::class_<UsdNprTokensType, pxr_boost::python::noncopyable>
        cls("Tokens", pxr_boost::python::no_init);
    _ADD_TOKEN(cls, boundaries);
    _ADD_TOKEN(cls, boundaryWidth);
    _ADD_TOKEN(cls, contourSurfaces);
    _ADD_TOKEN(cls, contourViewPoint);
    _ADD_TOKEN(cls, creases);
    _ADD_TOKEN(cls, creaseWidth);
    _ADD_TOKEN(cls, drawBoundary);
    _ADD_TOKEN(cls, drawCrease);
    _ADD_TOKEN(cls, drawSilhouette);
    _ADD_TOKEN(cls, silhouettes);
    _ADD_TOKEN(cls, silhouetteWidth);
    _ADD_TOKEN(cls, surfaces);
    _ADD_TOKEN(cls, Contour);
}
