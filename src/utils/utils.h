#pragma once

#include <dirent.h>
#include "../default.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>

PXR_NAMESPACE_OPEN_SCOPE

#define RANDOM_0_1 ((float)rand() / (float)RAND_MAX)

#define RANDOM_0_X(HI) ((float)rand() / (float) RAND_MAX * (HI))

#define RANDOM_LO_HI(LO, HI) ((LO) + (float)rand() / \
  (float)(RAND_MAX / ((HI) - (LO))))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(a, min, max) ((a)<(min)?(min):(a)>(max)?(max):(a))
#define RESCALE(value, inmin, inmax, outmin, outmax) \
  (((value) - (inmin))*((outmax)-(outmin))/((inmax)-(inmin))+(outmin))

// x=target variable, y=mask
#define BITMASK_SET(x,y) ((x) |= (y))
#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK(x,y) (((x) & (y)) == (y))

// print vectors (debug)
void PrintVector(const pxr::GfVec2i& v, const char* t);
void PrintVector(const pxr::GfVec3f& v, const char* t);
void PrintVector(const pxr::GfVec4f& v, const char* t);

// index to random color
static inline unsigned RandomColorByIndex(unsigned index)
{
  srand(index);
  return rand();
}

static inline int PackColor(float r, float g, float b, float a)
{
  return 0;
}

static inline pxr::GfVec4f UnpackColor(uint32_t packed)
{
  uint8_t alpha = (packed & 0xFF000000) >> 24;
  uint8_t blue = (packed & 0x00FF0000) >> 16;
  uint8_t green = (packed & 0x0000FF00) >> 8;
  uint8_t red = (packed & 0x000000FF);

  return pxr::GfVec4f(
    (float)red/255.f, 
    (float)green/255.f, 
    (float)blue/255.f, 
    (float)alpha/255.f
  );
}

// color for procedural GL textures
union GLColor 
{
  int32_t  packed;
  struct {
    char r;
    char g;
    char b;
    char a;
  } components;
};

// num files in directory
static int FilesInDirectory()
{
  DIR *dir;
  struct dirent *ent;
  int num_files = 2;
  if ((dir = opendir ("/Users/benmalartre/Documents/RnD/embree/embree-usd/images")) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      printf ("%s\n", ent->d_name);
      num_files++;
    }
    closedir (dir);
    return num_files - 2;
  } else {
    /* could not open directory */
    perror ("");
    return EXIT_FAILURE;
  }
}
 
PXR_NAMESPACE_CLOSE_SCOPE