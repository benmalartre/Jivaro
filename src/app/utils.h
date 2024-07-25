#include <vector>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

// Look for a new token. If prefix ends with a number, it will increase its value until
// a valid token is found
std::string FindNextAvailableTokenString(std::string prefix);

// Find usd file format extensions and returns them prefixed with a dot
const std::vector<std::string> GetUsdValidExtensions();

JVR_NAMESPACE_CLOSE_SCOPE