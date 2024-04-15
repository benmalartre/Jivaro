#include "../pbd/mask.h"
#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

void Mask::SetMask(const std::vector<int>& used)
{
  const size_t numBytes = used.size() / sizeof(int) + 1;
  _mask.resize(numBytes, 0);

  for (size_t index = 0; index < used.size(); ++index) {
    if (!used[index]) continue;
    BITMASK_SET(_mask[index / sizeof(int)], index % sizeof(int));
  }
}

void Mask::RemoveMask()
{
  _mask.clear();
}

JVR_NAMESPACE_CLOSE_SCOPE