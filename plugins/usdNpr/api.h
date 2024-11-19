//
// Copyright 2017 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDNPR_API_H
#define USDNPR_API_H

#include "pxr/base/arch/export.h"

#if defined(PXR_STATIC)
#   define USDNPR_API
#   define USDNPR_API_TEMPLATE_CLASS(...)
#   define USDNPR_API_TEMPLATE_STRUCT(...)
#   define USDNPR_LOCAL
#else
#   if defined(USDNPR_EXPORTS)
#       define USDNPR_API ARCH_EXPORT
#       define USDNPR_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDNPR_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#   else
#       define USDNPR_API ARCH_IMPORT
#       define USDNPR_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDNPR_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#   endif
#   define USDNPR_LOCAL ARCH_HIDDEN
#endif

#endif
