#include "../ui/viewport.h"
#include "../app/camera.h"
#include "../app/tools.h"
#include "../app/selection.h"
#include "../app/engine.h"
#include "../app/application.h"

#include <pxr/imaging/cameraUtil/conformWindow.h>

PXR_NAMESPACE_OPEN_SCOPE

Tool::Tool()
  : _translate(TranslateHandle())
  , _rotate(RotateHandle())
  , _scale(ScaleHandle())
  , _brush(BrushHandle())
  , _interacting(false)
  , _active(NULL)
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
  _brush.Setup();
  SetActiveTool(TOOL_TRANSLATE);
}


void Tool::SetViewport(const pxr::GfVec4f& viewport)
{
  _viewport = viewport;
}

void Tool::SetCamera(Camera* camera)
{
  _translate.SetCamera(camera);
  _rotate.SetCamera(camera);
  _scale.SetCamera(camera);
  _brush.SetCamera(camera);

  pxr::GfMatrix4d conformWindowProjectionMatrix =
    CameraUtilConformedWindow(
      camera->GetProjectionMatrix(),
      CameraUtilConformWindowPolicy::CameraUtilFit, _viewport[2]/_viewport[3]);

  // update shader
  Shape::UpdateCamera(
    pxr::GfMatrix4f(camera->GetViewMatrix()),
    pxr::GfMatrix4f(conformWindowProjectionMatrix));

  if(_active) {
    _active->UpdatePickingPlane(_active->GetActiveAxis());
  }
}

void Tool::SetActiveTool(short tool)
{
  switch(tool) {
    case TOOL_NONE:
      _active = NULL;
      break;
    case TOOL_BRUSH:
      _active = (BaseHandle*)&_brush;
      break;
    case TOOL_SCALE:
      _active = (BaseHandle*)&_scale;
      break;
    case TOOL_ROTATE:
      _active = (BaseHandle*)&_rotate;
      break;
    case TOOL_TRANSLATE:
      _active = (BaseHandle*)&_translate;
      break;
    default:
      _active = NULL;
      break;
  }
  if(_active)_active->ResetSelection();
}

bool Tool::IsActive()
{
  return (_active != NULL);
}

bool Tool::IsInteracting() {
  if (_active && _interacting) {
    return true;
  }
  else {
    return false;
  }
}

void Tool::Draw(float width, float height)
{
  Selection* selection = GetApplication()->GetSelection();
  if(_active && selection->GetNumSelectedItems()) {
    _active->Draw(width ,height);
  }
}

void Tool::Select(float x, float y, float width, float height, bool lock)
{
  Selection* selection = GetApplication()->GetSelection();
  if(_active  && selection->GetNumSelectedItems()) {
    _activeAxis = _active->Select(x, y, width, height, lock);
  }
}

void Tool::Pick(float x, float y, float width, float height)
{
  if(_active) {
    _hoveredAxis = _active->Pick(x, y, width, height);
  }
}

void Tool::BeginUpdate(float x, float y, float width, float height)
{
  Selection* selection = GetApplication()->GetSelection();
  if(_active && selection->GetNumSelectedItems()) {
    if (_activeAxis != BaseHandle::AXIS_NONE) {
      _active->BeginUpdate(x, y, width, height);
      _interacting = true;
    }
    else {
      _interacting = false;
    }
  }
}

void Tool::EndUpdate(float x, float y, float width, float height)
{
  Pick(x, y, width, height);
  if(_active) {
    _active->EndUpdate();
  }
  _interacting = false;
}

void Tool::Update(float x, float y, float width, float height)
{
  Application* app = GetApplication();
  Selection* selection = app->GetSelection();
  if(_active && selection->GetNumSelectedItems()) {
    _active->Update(x, y, width, height);
  }
}

void Tool::ResetSelection()
{
  if(_active) {
    _active->ResetSelection();
  }
}

void InitializeTools()
{
   InitShapeShader();
}


PXR_NAMESPACE_CLOSE_SCOPE
