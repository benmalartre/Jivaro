#ifndef JVR_APPLICATION_CALLBACKS_H
#define JVR_APPLICATION_CALLBACKS_H

#include <pxr/usd/sdf/path.h>


#include "../command/command.h"
#include "../command/inverse.h"
#include "../command/block.h"

#include "../app/selection.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace Callbacks {
  void RenamePrim(Model* model, const SdfPath& path, const TfToken& token);
  void CreatePrim(Model* model, const TfToken& type);
  void DeletePrim(Model* model, const SdfPath& path);
}
JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_CALLBACKS_H