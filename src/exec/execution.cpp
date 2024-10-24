#include <GLFW/glfw3.h>
#include <pxr/usd/usdGeom/xform.h>
#include "../exec/execution.h"

JVR_NAMESPACE_OPEN_SCOPE

void Execution::_GetRootPrim(pxr::UsdStageRefPtr& stage)
{
  // get root prim
  _rootPrim = stage->GetDefaultPrim();
  if(!_rootPrim.IsValid()) {
    pxr::UsdGeomXform root = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/Root"));
    _rootPrim = root.GetPrim();
    stage->SetDefaultPrim(_rootPrim);
  }
  _rootId = _rootPrim.GetPath();
};


void Execution::ViewEvent(const ExecViewEventData& data)
{
  switch(data.type) {
    case ExecViewEventData::MOUSE_BUTTON:
      _MouseButtonEvent(data);
      break;

    case ExecViewEventData::MOUSE_MOVE:
      _MouseMoveEvent(data);
      break;

    case ExecViewEventData::KEYBOARD_INPUT:
      _KeyboardInputEvent(data);
      break;
  }
}

void Execution::_MouseButtonEvent(const ExecViewEventData &data)
{
  std::cout << "EXEC MOUSE BUTTON EVENT : " << data.button << " : " << " was ";
  switch(data.action) {
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

void Execution::_MouseMoveEvent(const ExecViewEventData &data)
{

}

void Execution::_KeyboardInputEvent(const ExecViewEventData &data)
{

}

JVR_NAMESPACE_CLOSE_SCOPE
