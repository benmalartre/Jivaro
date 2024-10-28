//
// Copyright 2023 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
////////////////////////////////////////////////////////////////////////

/* ************************************************************************** */
/* **                                                                      ** */
/* ** This file is generated by a script.                                  ** */
/* **                                                                      ** */
/* ** Do not edit it directly (unless it is within a CUSTOM CODE section)! ** */
/* ** Edit hdSchemaDefs.py instead to make changes.                        ** */
/* **                                                                      ** */
/* ************************************************************************** */

#include "pxr/imaging/hd/extComputationInputComputationSchema.h"

#include "pxr/imaging/hd/retainedDataSource.h"

#include "pxr/base/trace/trace.h"

// --(BEGIN CUSTOM CODE: Includes)--
// --(END CUSTOM CODE: Includes)--

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(HdExtComputationInputComputationSchemaTokens,
    HD_EXT_COMPUTATION_INPUT_COMPUTATION_SCHEMA_TOKENS);

// --(BEGIN CUSTOM CODE: Schema Methods)--
// --(END CUSTOM CODE: Schema Methods)--

HdPathDataSourceHandle
HdExtComputationInputComputationSchema::GetSourceComputation() const
{
    return _GetTypedDataSource<HdPathDataSource>(
        HdExtComputationInputComputationSchemaTokens->sourceComputation);
}

HdTokenDataSourceHandle
HdExtComputationInputComputationSchema::GetSourceComputationOutputName() const
{
    return _GetTypedDataSource<HdTokenDataSource>(
        HdExtComputationInputComputationSchemaTokens->sourceComputationOutputName);
}

/*static*/
HdContainerDataSourceHandle
HdExtComputationInputComputationSchema::BuildRetained(
        const HdPathDataSourceHandle &sourceComputation,
        const HdTokenDataSourceHandle &sourceComputationOutputName
)
{
    TfToken _names[2];
    HdDataSourceBaseHandle _values[2];

    size_t _count = 0;

    if (sourceComputation) {
        _names[_count] = HdExtComputationInputComputationSchemaTokens->sourceComputation;
        _values[_count++] = sourceComputation;
    }

    if (sourceComputationOutputName) {
        _names[_count] = HdExtComputationInputComputationSchemaTokens->sourceComputationOutputName;
        _values[_count++] = sourceComputationOutputName;
    }
    return HdRetainedContainerDataSource::New(_count, _names, _values);
}

HdExtComputationInputComputationSchema::Builder &
HdExtComputationInputComputationSchema::Builder::SetSourceComputation(
    const HdPathDataSourceHandle &sourceComputation)
{
    _sourceComputation = sourceComputation;
    return *this;
}

HdExtComputationInputComputationSchema::Builder &
HdExtComputationInputComputationSchema::Builder::SetSourceComputationOutputName(
    const HdTokenDataSourceHandle &sourceComputationOutputName)
{
    _sourceComputationOutputName = sourceComputationOutputName;
    return *this;
}

HdContainerDataSourceHandle
HdExtComputationInputComputationSchema::Builder::Build()
{
    return HdExtComputationInputComputationSchema::BuildRetained(
        _sourceComputation,
        _sourceComputationOutputName
    );
} 

PXR_NAMESPACE_CLOSE_SCOPE