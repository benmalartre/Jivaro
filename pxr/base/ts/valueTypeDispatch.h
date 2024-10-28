//
// Copyright 2024 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#ifndef PXR_BASE_TS_VALUE_TYPE_DISPATCH_H
#define PXR_BASE_TS_VALUE_TYPE_DISPATCH_H

#include "pxr/pxr.h"
#include "pxr/base/ts/typeHelpers.h"
#include "pxr/base/gf/half.h"
#include "pxr/base/tf/type.h"
#include "pxr/base/tf/diagnostic.h"

#include <utility>

PXR_NAMESPACE_OPEN_SCOPE


// Makes a call to a template functor based on a dynamic type.  No return value;
// obtain outputs with out-params.  Supports all valid spline value types.
//
// Example:
//
//   template <typename T>
//   struct _HasNonzeroValue
//   {
//       void operator()(const TsKnot &knot, bool *resultOut)
//       {
//           T value = 0;
//           if (knot.GetValue(&value))
//               *resultOut = (value != 0);
//           else
//               *resultOut = false;
//       }
//   };
//
//   bool nonzero = false;
//   TsDispatchToValueTypeTemplate<_HasNonzeroValue>(
//       myKnot.GetValueType(), myKnot, &nonzero);
//
template <
    template <typename T> class Cls,
    typename... Args>
void TsDispatchToValueTypeTemplate(
    TfType valueType, Args&&... args)
{
    if (valueType == Ts_GetType<double>())
    {
        Cls<double>()(std::forward<Args>(args)...);
    }
    else if (valueType == Ts_GetType<float>())
    {
        Cls<float>()(std::forward<Args>(args)...);
    }
    else if (valueType == Ts_GetType<GfHalf>())
    {
        Cls<GfHalf>()(std::forward<Args>(args)...);
    }
    else
    {
        TF_CODING_ERROR("Unsupported spline value type");
    }
}


PXR_NAMESPACE_CLOSE_SCOPE

#endif
