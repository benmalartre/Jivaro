
#include "pxr/pxr.h"
#include "pxr/base/tf/type.h"
#include "pxr/base/tf/registryManager.h"
#include "../app/notice.h"

PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<JVR::BaseNotice, TfType::Bases<TfNotice> >();
  TfType::Define<JVR::NewSceneNotice, TfType::Bases<JVR::BaseNotice> >();
  TfType::Define<JVR::SelectionChangedNotice, TfType::Bases<JVR::BaseNotice> >();
}


JVR_NAMESPACE_OPEN_SCOPE

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

JVR_NAMESPACE_CLOSE_SCOPE