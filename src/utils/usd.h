#ifndef JVR_UTILS_USD_H
#define JVR_UTILS_USD_H

#include <array>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/sdf/valueTypeName.h>
#include <pxr/usd/sdf/types.h>
#include <pxr/usd/usd/typed.h>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

// Return all the value types.
// In the next version 22.03 we should be able to use:
// SdfSchema::GetInstance().GetAllTypes();
const std::array<SdfValueTypeName, 106> & GetAllValueTypeNames();

// Return all the prim types from the registry.
// There is no function in USD to simply retrieve the list, this is explained in the forum:
// https://groups.google.com/g/usd-interest/c/q8asqMYuyeg/m/sRhFTIEfCAAJ
const std::vector<std::string> &GetAllSpecTypeNames();

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UTILS_USD_H