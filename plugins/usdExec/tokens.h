//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDEXEC_TOKENS_H
#define USDEXEC_TOKENS_H

/// \file usdExec/tokens.h

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// This is an automatically generated file (by usdGenSchema.py).
// Do not hand-edit!
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "pxr/pxr.h"
#include "pxr/usd/usdExec/api.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/tf/token.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE


/// \class UsdExecTokensType
///
/// \link UsdExecTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// These tokens are auto-generated from the module's schema, representing
/// property names, for when you need to fetch an attribute or relationship
/// directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
/// manner, and allow the compiler to verify that you spelled the name
/// correctly.
///
/// UsdExecTokens also contains all of the \em allowedTokens values
/// declared for schema builtin attributes of 'token' scene description type.
/// Use UsdExecTokens like so:
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(UsdExecTokens->execMetadata);
/// \endcode
struct UsdExecTokensType {
    USDEXEC_API UsdExecTokensType();
    /// \brief "execMetadata"
    /// 
    /// Dictionary valued metadata key authored on
    const TfToken execMetadata;
    /// \brief "full"
    /// 
    /// Possible value for 'connectability' metadata on
    const TfToken full;
    /// \brief "inputs:"
    /// 
    /// The prefix on exec attributes
    const TfToken inputs;
    /// \brief "interfaceOnly"
    /// 
    /// Possible value for 'connectability' metadata on 
    const TfToken interfaceOnly;
    /// \brief "outputs:"
    /// 
    /// The prefix on exec attributes
    const TfToken outputs;
    /// \brief "ExecConnectableAPI"
    /// 
    /// Schema identifer and family for UsdExecConnectableAPI
    const TfToken ExecConnectableAPI;
    /// \brief "ExecGraph"
    /// 
    /// Schema identifer and family for UsdExecGraph
    const TfToken ExecGraph;
    /// \brief "ExecNode"
    /// 
    /// Schema identifer and family for UsdExecNode
    const TfToken ExecNode;
    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var UsdExecTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa UsdExecTokensType
extern USDEXEC_API TfStaticData<UsdExecTokensType> UsdExecTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif