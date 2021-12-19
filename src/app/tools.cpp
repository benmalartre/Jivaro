#include "../ui/viewport.h"
#include "../app/camera.h"
#include "../app/tools.h"
#include "../app/selection.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE

Tool::Tool()
  : _translate(TranslateHandle())
  , _rotate(RotateHandle())
  , _scale(ScaleHandle())
  , _brush(BrushHandle())
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
  _brush.Setup();
  SetActiveTool(TOOL_TRANSLATE);
}


void Tool::SetViewport(ViewportUI* viewport)
{
  _viewport=viewport;
  Camera* camera = viewport->GetCamera();
  _translate.SetCamera(camera);
  _rotate.SetCamera(camera);
  _scale.SetCamera(camera);
  _brush.SetCamera(camera);

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

void Tool::Draw()
{
  Selection* selection = APPLICATION->GetSelection();
  if(_active && _viewport && selection->GetNumSelectedItems()) {
    _active->Draw(_viewport->GetWidth(), _viewport->GetHeight());
  }
}

void Tool::Select(bool lock)
{
  Selection* selection = APPLICATION->GetSelection();
  if(_active && _viewport && selection->GetNumSelectedItems()) {
    _activeAxis = _active->Select(
      _viewport->GetLastMouseX() - _viewport->GetX(),
      _viewport->GetLastMouseY() - _viewport->GetY(),
      _viewport->GetWidth(),
      _viewport->GetHeight(), lock);
  }
}

void Tool::Pick()
{
  if(_active && _viewport) {
    _hoveredAxis = _active->Pick(
      _viewport->GetLastMouseX() - _viewport->GetX(),
      _viewport->GetLastMouseY() - _viewport->GetY(),
      _viewport->GetWidth(),
      _viewport->GetHeight());
  }
}

void Tool::BeginUpdate()
{
  Selection* selection = APPLICATION->GetSelection();
  if(_active && _viewport && selection->GetNumSelectedItems()) {
    if (_activeAxis != BaseHandle::AXIS_NONE) {
      _active->BeginUpdate(
        _viewport->GetLastMouseX() - _viewport->GetX(),
        _viewport->GetLastMouseY() - _viewport->GetY(),
        _viewport->GetWidth(),
        _viewport->GetHeight());
      _interacting = true;
    }
    else {
      _interacting = false;
    }
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
  Selection* selection = APPLICATION->GetSelection();
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
  if(_active) {
    _active->ResetSelection();
  }
}

void InitializeTools()
{
   InitShapeShader();
}


JVR_NAMESPACE_CLOSE_SCOPE
