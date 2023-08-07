#pragma once
#include <cassert>
#include <pxr/usd/sdf/listEditorProxy.h>
#include <pxr/usd/sdf/reference.h>
#include <pxr/usd/sdf/listOp.h>

PXR_NAMESPACE_USING_DIRECTIVE

// Look for a new token. If prefix ends with a number, it will increase its value until
// a valid token is found
std::string FindNextAvailableTokenString(std::string prefix);

// Find usd file format extensions and returns them prefixed with a dot
const std::vector<std::string> GetUsdValidExtensions();