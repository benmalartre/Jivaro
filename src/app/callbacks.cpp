
#include "../command/manager.h"
#include "../ui/ui.h"
#include "../utils/strings.h"
#include "../app/commands.h"
#include "../app/callbacks.h"
#include "../app/notice.h"
#include "../app/model.h"
#include "../app/index.h"
#include "../app/registry.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE

namespace Callbacks {
  void RenamePrim(Model* model, const SdfPath& path, const TfToken& token)
  {
    if(path.IsEmpty() || token.IsEmpty())  return;

    UsdPrim prim = model->GetStage()->GetPrimAtPath(path);
    if(!prim.IsValid() || token.IsEmpty())
      return;

    ADD_COMMAND(RenamePrimCommand, model->GetRootLayer(), path, token);
  }

  void CreatePrim(Model* model, const TfToken& type)
  {
    Selection* selection = model->GetSelection();
    TfToken name(type.GetString() + "_" + RandomString(6));

    if (selection->GetNumSelectedItems()) {
      ADD_COMMAND(CreatePrimCommand, model->GetRootLayer(), selection->GetItem(0).path.AppendChild(name), type);
    }
    else {
      ADD_COMMAND(CreatePrimCommand, model->GetRootLayer(), SdfPath("/" + name.GetString()), type);
    }
  }

  void DeletePrim(Model* model, const SdfPath& path)
  {
    ADD_COMMAND(DeletePrimCommand, model->GetRootLayer(), {path});
  }


  void SetActiveTool(short tool)
  {
    WindowRegistry* registry = WindowRegistry::Get();
    registry->SetActiveTool(tool);
    ToolChangedNotice(tool).Send();
  }

  void ToggleExec()
  {
    Application::Get()->GetIndex()->ToggleExec();
  }
  
}

JVR_NAMESPACE_CLOSE_SCOPE
