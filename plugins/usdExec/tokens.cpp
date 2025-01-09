//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "pxr/usd/usdExec/tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

UsdExecTokensType::UsdExecTokensType() :
    execMetadata("execMetadata", TfToken::Immortal),
    full("full", TfToken::Immortal),
    inputs("inputs:", TfToken::Immortal),
    interfaceOnly("interfaceOnly", TfToken::Immortal),
    outputs("outputs:", TfToken::Immortal),
    ExecConnectableAPI("ExecConnectableAPI", TfToken::Immortal),
    ExecGraph("ExecGraph", TfToken::Immortal),
    ExecNode("ExecNode", TfToken::Immortal),
    allTokens({
        execMetadata,
        full,
        inputs,
        interfaceOnly,
        outputs,
        ExecConnectableAPI,
        ExecGraph,
        ExecNode
    })
{
}

TfStaticData<UsdExecTokensType> UsdExecTokens;

PXR_NAMESPACE_CLOSE_SCOPE
