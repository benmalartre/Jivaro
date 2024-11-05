#ifndef JVR_APPLICATION_CALLBACKS_H
#define JVR_APPLICATION_CALLBACKS_H

#include <pxr/usd/sdf/path.h>

JVR_NAMESPACE_OPEN_SCOPE

class Model;
namespace Callbacks {
  void CreatePrim(Model* model, const TfToken& type);
  void DeletePrim(Model* model, const SdfPath& path);
  void RenamePrim(Model* model, const SdfPath& path, const TfToken& token);

}
JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_CALLBACKS_H