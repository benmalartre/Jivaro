#ifndef AMN_UI_API_H
#define AMN_UI_API_H

#include <pxr/base/arch/export.h>

#if defined(PXR_STATIC)
#   define AMNUI_API
#   define AMNUI_API_TEMPLATE_CLASS(...)
#   define AMNUI_API_TEMPLATE_STRUCT(...)
#   define AMNUI_LOCAL
#else
#   if defined(AMNUI_EXPORTS)
#       define AMNUI_API ARCH_EXPORT
#       define AMNUI_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#       define AMNUI_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#   else
#       define AMNUI_API ARCH_IMPORT
#       define AMNUI_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#       define AMNUI_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#   endif
#   define AMNUI_LOCAL ARCH_HIDDEN
#endif

#endif // AMN_API_H
