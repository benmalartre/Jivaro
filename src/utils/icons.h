#ifndef AMN_UTILS_ICONS_H
#define AMN_UTILS_ICONS_H

#pragma once
#include <map>
#include <type_traits>
#include "../common.h"
#include "files.h"
#include "glutils.h"
#include "pxr/usd/ar/asset.h"
#include "pxr/usd/ar/resolver.h"
#include <pxr/imaging/hio/image.h>
#include <pxr/imaging/hio/stb_image.h>
#include <pxr/imaging/hio/stb_image_resize.h>

AMN_NAMESPACE_OPEN_SCOPE
enum ICON_SIZE {
  AMN_ICON_SMALL,
  AMN_ICON_MEDIUM,
  AMN_ICON_LARGE
};

enum ICON_ID {
  ICON_VISIBLE,
  ICON_INVISIBLE,
  ICON_PLAYBACK_FORWARD,
  ICON_PLAYBACK_BACKWARD,
  ICON_FIRST_FRAME,
  ICON_PREVIOUS_FRAME,
  ICON_NEXT_FRAME,
  ICON_LAST_FRAME,
  ICON_STOP_PLAYBACK,
  ICON_TRANSLATE,
  ICON_ROTATE,
  ICON_SCALE,
  ICON_MAX_ID
};
static const char* ICON_NAMES[ICON_MAX_ID] = {
  "visible",
  "invisible",
  "playforward",
  "playbackward",
  "firstframe",
  "lastframe",
  "stop",
  "translate",
  "rotate",
  "scale"
};

#define ICON_INTERNAL_FORMAT GL_RGBA
#define ICON_FORMAT GL_RGBA
#define ICON_TYPE GL_UNSIGNED_BYTE

struct Icon {
  size_t        size;
  GLuint        tex;
  GLuint        tex_h;
};
 
typedef std::vector<std::map<std::string, Icon> > AmnIconMap;
extern AmnIconMap AMN_ICONS;

void IconHoverDatas(pxr::HioImage::StorageSpec* storage, int nchannels);
void CreateIconFromImage(const std::string& filename,
  const std::string& name, ICON_SIZE size);
void AMNInitializeIcons();
void AMNTerminateIcons();

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UTILS_ICONS_H