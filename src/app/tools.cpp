#include "tools.h"
#include "../ui/viewport.h"
#include "../app/camera.h"
#include "../app/selection.h"
#include "../app/application.h"

AMN_NAMESPACE_OPEN_SCOPE

Tool::Tool()
  : _translate(TranslateHandle())
  , _rotate(RotateHandle())
  , _scale(ScaleHandle())
  , _interacting(false)
  , _active(NULL)
  , _viewport(NULL)
{

}

Tool::~Tool()
{ 
}

void Tool::Init()
{

  _translate.Setup();
  _rotate.Setup();
  _scale.Setup();
  SetActiveTool(AMN_TOOL_TRANSLATE);
}


void Tool::SetViewport(ViewportUI* viewport)
{
  _viewport=viewport;
  Camera* camera = viewport->GetCamera();
  _translate.SetCamera(camera);
  _rotate.SetCamera(camera);
  _scale.SetCamera(camera);

  // update shader
  Shape::UpdateCamera(
    pxr::GfMatrix4f(camera->GetViewMatrix()),
    pxr::GfMatrix4f(camera->GetProjectionMatrix()));

  if(_active) {
    _active->UpdatePickingPlane(_active->GetActiveAxis());
  }
}

void Tool::SetActiveTool(short tool)
{
  switch(tool) {
    case AMN_TOOL_BRUSH:
    case AMN_TOOL_NONE:
      _active = NULL;
      break;
    case AMN_TOOL_SCALE:
      _active = (BaseHandle*)&_scale;
      break;
    case AMN_TOOL_ROTATE:
      _active = (BaseHandle*)&_rotate;
      break;
    case AMN_TOOL_TRANSLATE:
      _active = (BaseHandle*)&_translate;
      break;
    default:
    _active = NULL;
    break;
  }
  if(_active)_active->ResetSelection();
}

void Tool::Draw()
{
  Selection* selection = AMN_APPLICATION->GetSelection();
  if(_active && _viewport && selection->GetNumSelectedItems()) {
    _active->Draw();
  }
}

void Tool::Select(bool lock)
{
  Selection* selection = AMN_APPLICATION->GetSelection();
  if(_active && _viewport && selection->GetNumSelectedItems()) {
    short activeAxis = _active->Select(
      _viewport->GetLastMouseX() - _viewport->GetX(),
      _viewport->GetLastMouseY() - _viewport->GetY(),
      _viewport->GetWidth(),
      _viewport->GetHeight(), lock);
  }
}

void Tool::Pick()
{
  if(_active && _viewport) {
    short hoveredAxis = _active->Pick(
      _viewport->GetLastMouseX() - _viewport->GetX(),
      _viewport->GetLastMouseY() - _viewport->GetY(),
      _viewport->GetWidth(),
      _viewport->GetHeight());
  }
}

void Tool::BeginUpdate()
{
  Selection* selection = AMN_APPLICATION->GetSelection();
  if(_active && _viewport && selection->GetNumSelectedItems()) {
    _active->BeginUpdate(
      _viewport->GetLastMouseX() - _viewport->GetX(),
      _viewport->GetLastMouseY() - _viewport->GetY(),
      _viewport->GetWidth(),
      _viewport->GetHeight());
    _interacting = true;
  }
}

void Tool::EndUpdate()
{
  Pick();
  if(_active && _viewport) {
    _active->EndUpdate();
  }
  _interacting = false;
}

void Tool::Update()
{
  Selection* selection = AMN_APPLICATION->GetSelection();
  if(_active && _viewport && selection->GetNumSelectedItems()) {
    _active->Update(
      _viewport->GetLastMouseX() - _viewport->GetX(),
      _viewport->GetLastMouseY() - _viewport->GetY(),
      _viewport->GetWidth(),
      _viewport->GetHeight());
  }
}

void Tool::ResetSelection()
{
  std::cout << "TOOL : RESET SELECTION = " << _active << std::endl;
  if(_active) {
    _active->ResetSelection();
  }
}

void AMNInitializeTools()
{
   AMNInitShapeShader();
}


AMN_NAMESPACE_CLOSE_SCOPE
