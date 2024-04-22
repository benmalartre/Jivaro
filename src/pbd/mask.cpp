#include "../pbd/mask.h"
#include "../pbd/particle.h"

JVR_NAMESPACE_OPEN_SCOPE

void Mask::SetMask(size_t n, const std::vector<int>& used)
{
  _mask.resize((n +1)/sizeof(int), 0);

  for(auto& index: used)
    BIT_SET(_mask[index / sizeof(int)], index % sizeof(int));

}

void Mask::RemoveMask()
{
  _mask.clear();
}


JVR_NAMESPACE_CLOSE_SCOPE