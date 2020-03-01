#include "utils.h"

AMN_NAMESPACE_OPEN_SCOPE

// print vectors (debug)
void
PrintVector(const pxr::GfVec2i& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] << std::endl;
}

void
PrintVector(const pxr::GfVec3f& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] 
    << "," << v[2] << std::endl;
}

void
PrintVector(const pxr::GfVec4f& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] 
    << "," << v[2] << "," << v[3] <<std::endl;
}

AMN_NAMESPACE_CLOSE_SCOPE