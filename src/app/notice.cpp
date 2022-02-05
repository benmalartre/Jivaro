
#include "pxr/pxr.h"
#include "pxr/base/tf/type.h"
#include "pxr/base/tf/registryManager.h"
#include "../app/notice.h"


PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<BaseNotice, TfType::Bases<TfNotice> >();
  TfType::Define<NewSceneNotice, TfType::Bases<BaseNotice> >();
  TfType::Define<SelectionChangedNotice, TfType::Bases<BaseNotice> >();
  TfType::Define<SceneChangedNotice, TfType::Bases<BaseNotice> >();
  TfType::Define<AttributeChangedNotice, TfType::Bases<BaseNotice> >();
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


PXR_NAMESPACE_CLOSE_SCOPE