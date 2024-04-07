#ifndef JVR_UI_API_H
#define JVR_UI_API_H

#include <pxr/base/arch/export.h>

#if defined(PXR_STATIC)
#   define UI_API
#   define UI_API_TEMPLATE_CLASS(...)
#   define UI_API_TEMPLATE_STRUCT(...)
#   define UI_LOCAL
#else
#   if defined(UI_EXPORTS)
#       define UI_API ARCH_EXPORT
#       define UI_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#       define UI_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#   else
#       define UI_API ARCH_IMPORT
#       define UI_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#       define UI_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#   endif
#   define UI_LOCAL ARCH_HIDDEN
#endif

#endif // JVR_API_H
