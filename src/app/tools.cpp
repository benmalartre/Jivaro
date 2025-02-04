#include "../ui/viewport.h"
#include "../app/handle.h"
#include "../app/camera.h"
#include "../app/tools.h"
#include "../app/selection.h"
#include "../app/engine.h"
#include "../app/application.h"


JVR_NAMESPACE_OPEN_SCOPE

Tool::Tool()
  : _interacting(false)
  , _active(nullptr)
  , _last(Tool::NONE)
  , _current(Tool::NONE)
{
}

Tool::~Tool()
{ 
  if (_active)delete _active;
}


void 
Tool::SetViewport(const GfVec4f& viewport)
{
  _viewport = viewport;
}

void
Tool::SetProgram(GLSLProgram* pgm)
{
  _pgm = pgm;
}

void 
Tool::SetCamera(Camera* camera)
{
  if (!_active) return;
  _active->SetCamera(camera);

  // update shader
  _active->UpdateCamera(
    GfMatrix4f(camera->GetViewMatrix()),
    GfMatrix4f(camera->GetProjectionMatrix()));

  if(_active) {
    _active->UpdatePickingPlane(_active->GetActiveAxis());
  }
}

void
Tool::_ResetActiveTool()
{
  if (_last != _current) {
    if (_active)delete _active;
    switch (_current) {
    case Tool::NONE:
      _active = NULL;
      break;
    case Tool::SELECT:
      _active = new SelectHandle();
      break;
    case Tool::BRUSH:
      _active = new BrushHandle();
      break;
    case Tool::SCALE:
      _active = new ScaleHandle();
      break;
    case Tool::ROTATE:
      _active = new RotateHandle();
      break;
    case Tool::TRANSLATE:
      _active = new TranslateHandle();
      break;
    default:
      _active = NULL;
      break;
    }
    if (_active) {
      _active->SetProgram(_pgm);
      _active->Setup();
      _active->ResetSelection();
    }
  }
  _last = _current;
}

void Tool::SetActiveTool(short tool)
{
  _last = _current;
  _current = tool;
  _ResetActiveTool();
}

bool Tool::IsActive()
{
  return (_current != Tool::NONE);
}

bool Tool::IsInteracting() {
  if (_active && _interacting) {
    return true;
  }
  else {
    return false;
  }
}

short Tool::GetActiveTool(){
  return _current;
}

void Tool::Draw()
{
  Selection* selection = Application::Get()->GetModel()->GetSelection();
  if(_active && selection->GetNumSelectedItems()) {
    glClear(GL_DEPTH_BUFFER_BIT);
    _active->Draw(_viewport[2] , _viewport[3]);
  }
}

void Tool::Select(float x, float y, float width, float height, bool lock)
{
  Selection* selection = Application::Get()->GetModel()->GetSelection();
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
  Selection* selection = Application::Get()->GetModel()->GetSelection();
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
  Application* app = Application::Get();
  Selection* selection = app->GetModel()->GetSelection();
  if(_active && selection->GetNumSelectedItems()) {
    _active->Update(x, y, width, height);
  }
}

void Tool::ResetSelection()
{
  if(_active != nullptr) {
    _active->ResetSelection();
  }
}


JVR_NAMESPACE_CLOSE_SCOPE
