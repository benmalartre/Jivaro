#ifndef JVR_PBD_API_H
#define JVR_PBD_API_H

#include "pxr/base/arch/export.h"

#if defined(JVR_STATIC)
#   define PBD_API
#   define PBD_API_TEMPLATE_CLASS(...)
#   define PBD_API_TEMPLATE_STRUCT(...)
#   define PBD_LOCAL
#else
#   if defined(PBD_EXPORTS)
#       define PBD_API ARCH_EXPORT
#       define PBD_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#       define PBD_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#   else
#       define PBD_API ARCH_IMPORT
#       define PBD_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#       define PBD_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#   endif
#   define PBD_LOCAL ARCH_HIDDEN
#endif

#endif // JVR_PBD_API_H
