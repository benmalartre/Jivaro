//
// Copyright 2017 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDEXEC_API_H
#define USDEXEC_API_H

#include "pxr/base/arch/export.h"

#if defined(PXR_STATIC)
#   define USDEXEC_API
#   define USDEXEC_API_TEMPLATE_CLASS(...)
#   define USDEXEC_API_TEMPLATE_STRUCT(...)
#   define USDEXEC_LOCAL
#else
#   if defined(USDEXEC_EXPORTS)
#       define USDEXEC_API ARCH_EXPORT
#       define USDEXEC_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDEXEC_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#   else
#       define USDEXEC_API ARCH_IMPORT
#       define USDEXEC_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDEXEC_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#   endif
#   define USDEXEC_LOCAL ARCH_HIDDEN
#endif

#endif
