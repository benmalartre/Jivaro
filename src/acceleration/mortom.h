#ifndef JVR_ACCELERATION_MORTOM_H
#define JVR_ACCELERATION_MORTOM_H

#include "../common.h"
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3i.h>

JVR_NAMESPACE_OPEN_SCOPE

#define MORTOM_LONG_BITS  21
#define MORTOM_MAX_L ((1 << (MORTOM_LONG_BITS)) - 1)


// CONVERSION
pxr::GfVec3d MortomToWorld(const pxr::GfRange3d& range, const pxr::GfVec3i& p);
pxr::GfVec3i WorldToMortom(const pxr::GfRange3d& range, const pxr::GfVec3d& p);
void ClampMortom(pxr::GfVec3i& p);

// ENCODING
uint32_t Encode2D(const pxr::GfVec2i& p);
uint64_t Encode3D(const pxr::GfVec3i& p);

// DECODING
pxr::GfVec2i Decode2D(uint32_t code);
pxr::GfVec3i Decode3D(uint64_t code);
 
JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_ACCELERATION_MORTOM_H
