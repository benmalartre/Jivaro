#include <GLFW/glfw3.h>
#include <pxr/usd/usdGeom/xform.h>
#include "../ui/ui.h"
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

void Execution::_GetRootPrim(UsdStageRefPtr &stage)
{
  // get root prim
  _rootPrim = stage->GetDefaultPrim();
  if(!_rootPrim.IsValid()) {
    UsdGeomXform root = UsdGeomXform::Define(stage, SdfPath("/Root"));
    _rootPrim = root.GetPrim();
    stage->SetDefaultPrim(_rootPrim);
  }
  _rootId = _rootPrim.GetPath();
};


void Execution::ViewEvent(const ViewEventData *data)
{
  switch(data->type) {
    case ViewEventData::MOUSE_BUTTON:
      _MouseButtonEvent(data);
      break;

    case ViewEventData::MOUSE_MOVE:
      _MouseMoveEvent(data);
      break;

    case ViewEventData::KEYBOARD_INPUT:
      _KeyboardInputEvent(data);
      break;
  }
}

void Execution::_MouseButtonEvent(const ViewEventData *data)
{
  std::cout << "EXEC MOUSE BUTTON EVENT : " << data->button << " : " << " was ";
  switch(data->action) {
    case GLFW_PRESS:
      std::cout << "pressed!" << std::endl;
      break;

    case GLFW_RELEASE:
      std::cout << "released!" << std::endl;
      break; 

    case GLFW_REPEAT:
      std::cout << "repeated!" << std::endl;
      break;
  }
}

void Execution::_MouseMoveEvent(const ViewEventData *data)
{

}

void Execution::_KeyboardInputEvent(const ViewEventData *data)
{

}

JVR_NAMESPACE_CLOSE_SCOPE
