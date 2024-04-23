
#include "pxr/pxr.h"
#include "pxr/base/tf/type.h"
#include "pxr/base/tf/registryManager.h"
#include "../app/notice.h"


JVR_NAMESPACE_OPEN_SCOPE

PXR_NAMESPACE_USING_DIRECTIVE
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<BaseNotice, TfType::Bases<TfNotice> >();
  TfType::Define<NewSceneNotice, TfType::Bases<BaseNotice> >();
  TfType::Define<SelectionChangedNotice, TfType::Bases<BaseNotice> >();
  TfType::Define<SceneChangedNotice, TfType::Bases<BaseNotice> >();
  TfType::Define<AttributeChangedNotice, TfType::Bases<BaseNotice> >();
  TfType::Define<UndoStackNotice, TfType::Bases<BaseNotice> >();
}


////////////////////////////////////////////////////////////
BaseNotice::BaseNotice()
{
}

BaseNotice::~BaseNotice()
{
}

////////////////////////////////////////////////////////////

NewSceneNotice::NewSceneNotice()
{
}

NewSceneNotice::~NewSceneNotice()
{
}

////////////////////////////////////////////////////////////


SelectionChangedNotice::SelectionChangedNotice()
{
}

SelectionChangedNotice::~SelectionChangedNotice()
{
}

////////////////////////////////////////////////////////////

SceneChangedNotice::SceneChangedNotice()
{
}

SceneChangedNotice::~SceneChangedNotice()
{
}

////////////////////////////////////////////////////////////

AttributeChangedNotice::AttributeChangedNotice()
{
}

AttributeChangedNotice::~AttributeChangedNotice()
{
}

////////////////////////////////////////////////////////////

TimeChangedNotice::TimeChangedNotice()
{
}

TimeChangedNotice::~TimeChangedNotice()
{
}

////////////////////////////////////////////////////////////

UndoStackNotice::UndoStackNotice()
{
}

UndoStackNotice::~UndoStackNotice()
{
}


JVR_NAMESPACE_CLOSE_SCOPE