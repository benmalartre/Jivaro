#ifndef JVR_UTILS_BITS_H
#define JVR_UTILS_BITS_H

#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE
template <typename T>
size_t _NumActiveBits(T num)
{
  static const int numToBits[16] = 
    { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

  if (0 == num)return numToBits[0];

  int nibble = num & 0xf;
  return numToBits[nibble] + _NumActiveBits(num >> 4);
}

template <typename T>
size_t GetNumActiveBits(std::vector<T>& data)
{
  size_t numActiveBits=0;
  for(size_t i=0; i< data.size(); ++i) {
    numActiveBits += _NumActiveBits(data[i]);
  }
  return numActiveBits;
}

#endif //JVR_UTILS_BITS_H