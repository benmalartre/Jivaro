#ifndef JVR_PBD_MASK_H
#define JVR_PBD_MASK_H

#include <vector>
#include <limits>

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

class Mask
{
public:
  static const size_t INVALID_INDEX = std::numeric_limits<int>::max();

  Mask();
  Mask(const std::vector<int>& used);

  bool HasMask() const { return _mask.size() > 0; };
  size_t MaskSize() const { return _mask.size(); };
  const int* GetMaskCPtr() const { return &_mask[0]; };
  void SetMask(const std::vector<int>& used);
  void RemoveMask();

  inline bool Affects(size_t index) const {
    if (!HasMask())return true;
    const size_t bitsIdx = index / sizeof(int);
    if (bitsIdx >= _mask.size())return false;
    return BITMASK_CHECK(_mask[bitsIdx], index % sizeof(int));
  }

protected:     
  std::vector<int> _mask;                     // bits vector encoding particles usage mask
  //pxr::VtArray<float>         _weights;     // if mask weights size must equals mask size 
                                              // else weights size must equals num particles
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_MASK_H
