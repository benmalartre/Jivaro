//
// Copyright 2017 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDPBD_API_H
#define USDPBD_API_H

#include "pxr/base/arch/export.h"

#if defined(PXR_STATIC)
#   define USDPBD_API
#   define USDPBD_API_TEMPLATE_CLASS(...)
#   define USDPBD_API_TEMPLATE_STRUCT(...)
#   define USDPBD_LOCAL
#else
#   if defined(USDPBD_EXPORTS)
#       define USDPBD_API ARCH_EXPORT
#       define USDPBD_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDPBD_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#   else
#       define USDPBD_API ARCH_IMPORT
#       define USDPBD_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#       define USDPBD_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#   endif
#   define USDPBD_LOCAL ARCH_HIDDEN
#endif

#endif
