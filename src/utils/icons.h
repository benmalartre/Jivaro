#ifndef JVR_UTILS_ICONS_H
#define JVR_UTILS_ICONS_H

#pragma once
#include <map>
#include <type_traits>

#include <pxr/usd/ar/asset.h>
#include <pxr/usd/ar/resolver.h>
#include <pxr/imaging/hio/image.h>

#include "../common.h"
#include "../stb/stb_image.h"
#include "../stb/stb_image_resize.h"
#include "../utils/files.h"
#include "../utils/glutils.h"

JVR_NAMESPACE_OPEN_SCOPE
enum ICON_SIZE {
  ICON_SIZE_SMALL,
  ICON_SIZE_MEDIUM,
  ICON_SIZE_LARGE
};

enum ICON_STATE {
  ICON_DEFAULT,
  ICON_SELECTED,
  ICON_DISABLED
};

static const char* ICON_SUFFIX[3] = {
  "default",
  "selected",
  "disabled"
};

enum ICON_ID {
  ICON_VISIBLE,
  ICON_INVISIBLE,
  ICON_PLAYBACK_FORWARD,
  ICON_PLAYBACK_BACKWARD,
  ICON_PLAYBACK_STOP,
  ICON_PREVIOUS_FRAME,
  ICON_NEXT_FRAME,
  ICON_FIRST_FRAME,
  ICON_LAST_FRAME,
  ICON_PLAYBACK_LOOP,
  ICON_TRANSLATE,
  ICON_ROTATE,
  ICON_SCALE,
  ICON_BRUSH,
  ICON_PEN,
  ICON_SELECT,
  ICON_SPLITV,
  ICON_SPLITH,
  ICON_LOCKED,
  ICON_UNLOCKED,
  ICON_OP,
  ICON_TRASH,
  ICON_STAGE,
  ICON_LAYER,
  ICON_FOLDER,
  ICON_FILE,
  ICON_HOME,
  ICON_BACK,
  ICON_OPEN,
  ICON_SAVE,
  ICON_WARNING,
  ICON_ERROR,
  ICON_OK,
  ICON_EXPENDED,
  ICON_CONNECTED,
  ICON_COLLAPSED,
  ICON_ARROWLEFT,
  ICON_ARROWRIGHT,
  ICON_ARROWUP,
  ICON_ARROWDOWN,
  ICON_MAX_ID
};

static const char* ICON_NAMES[ICON_MAX_ID] = {
  "visible",
  "invisible",
  "playforward",
  "playbackward",
  "stop",
  "previousframe",
  "nextframe",
  "firstframe",
  "lastframe",
  "loop",
  "translate",
  "rotate",
  "scale",
  "brush",
  "pen",
  "select",
  "splitv",
  "splith",
  "locked",
  "unlocked",
  "op",
  "trash",
  "stage",
  "layer",
  "folder",
  "file",
  "home",
  "back",
  "open",
  "save",
  "warning",
  "error",
  "ok",
  "expended",
  "connected",
  "collapsed",
  "arrowleft",
  "arrowright",
  "arrowup",
  "arrowdown"
};

#define ICON_INTERNAL_FORMAT GL_RGBA
#define ICON_FORMAT GL_RGBA
#define ICON_TYPE GL_UNSIGNED_BYTE

struct Icon {
  size_t        size;
  GLuint        tex[3];
};
 
typedef std::vector<std::vector<Icon> > IconList;
extern IconList ICONS;

void IconHoverDatas(pxr::HioImage::StorageSpec* storage, int numChannels);
GLuint CreateIconFromImage(const std::string& filename, int index, ICON_SIZE size);
void InitializeIcons();
void TerminateIcons();

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UTILS_ICONS_H