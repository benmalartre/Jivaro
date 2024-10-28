//
// Copyright 2023 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef EXT_RMANPKG_25_0_PLUGIN_RENDERMAN_PLUGIN_HD_PRMAN_PORTAL_LIGHT_RESOLVING_SCENE_INDEX_PLUGIN_H
#define EXT_RMANPKG_25_0_PLUGIN_RENDERMAN_PLUGIN_HD_PRMAN_PORTAL_LIGHT_RESOLVING_SCENE_INDEX_PLUGIN_H

#include "pxr/pxr.h"
#include "pxr/imaging/hd/sceneIndexPlugin.h"
#include "hdPrman/api.h"

PXR_NAMESPACE_OPEN_SCOPE

/// \class HdPrman_PortalLightResolvingSceneIndexPlugin
///
/// Prman scene index plugin to resolve portal lights.
///
class HdPrman_PortalLightResolvingSceneIndexPlugin : public HdSceneIndexPlugin
{
public:
    HdPrman_PortalLightResolvingSceneIndexPlugin();

protected:
    HdSceneIndexBaseRefPtr _AppendSceneIndex(
        const HdSceneIndexBaseRefPtr& inputScene,
        const HdContainerDataSourceHandle& inputArgs) override;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // EXT_RMANPKG_25_0_PLUGIN_RENDERMAN_PLUGIN_HD_PRMAN_PORTAL_LIGHT_RESOLVING_SCENE_INDEX_PLUGIN_H
