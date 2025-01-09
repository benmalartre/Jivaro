//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// GENERATED FILE.  DO NOT EDIT.
#include "pxr/external/boost/python/class.hpp"
#include "pxr/usd/usdExec/tokens.h"

PXR_NAMESPACE_USING_DIRECTIVE

#define _ADD_TOKEN(cls, name) \
    cls.add_static_property(#name, +[]() { return UsdExecTokens->name.GetString(); });

void wrapUsdExecTokens()
{
    pxr_boost::python::class_<UsdExecTokensType, pxr_boost::python::noncopyable>
        cls("Tokens", pxr_boost::python::no_init);
    _ADD_TOKEN(cls, execMetadata);
    _ADD_TOKEN(cls, full);
    _ADD_TOKEN(cls, inputs);
    _ADD_TOKEN(cls, interfaceOnly);
    _ADD_TOKEN(cls, outputs);
    _ADD_TOKEN(cls, ExecConnectableAPI);
    _ADD_TOKEN(cls, ExecGraph);
    _ADD_TOKEN(cls, ExecNode);
}
