//
// Copyright 2020 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "pxr/pxr.h"
#include "pxr/imaging/pxOsd/tokens.h"

#include "pxr/base/tf/pyStaticTokens.h"

PXR_NAMESPACE_USING_DIRECTIVE

void wrapTokens()
{
    TF_PY_WRAP_PUBLIC_TOKENS(
        "OpenSubdivTokens",
        PxOsdOpenSubdivTokens, PXOSD_OPENSUBDIV_TOKENS);
}

