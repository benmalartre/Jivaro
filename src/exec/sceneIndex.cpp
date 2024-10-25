#include <pxr/base/vt/value.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/imaging/hd/xformSchema.h>

#include "../exec/sceneIndex.h"

JVR_NAMESPACE_OPEN_SCOPE

ExecSceneIndex::ExecSceneIndex(
    const HdSceneIndexBaseRefPtr &inputSceneIndex)
    : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
}

/*
GfMatrix4d ExecSceneIndex::GetXform(const SdfPath &primPath) const
{
    const VtValue *value = _xformDict.GetValueAtPath(primPath.GetAsString());
    if (value && !value->IsEmpty()) return value->Get<GfMatrix4d>();

    HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

    HdXformSchema xformSchema = HdXformSchema::GetFromParent(prim.dataSource);
    if (!xformSchema.IsDefined()) return GfMatrix4d(1);

    HdSampledDataSource::Time time(0);
    GfMatrix4d xform =
        xformSchema.GetMatrix()->GetValue(time).Get<GfMatrix4d>();

    return xform;
}

void ExecSceneIndex::SetXform(const SdfPath &primPath, GfMatrix4d xform)
{
    _xformDict.SetValueAtPath(primPath.GetAsString(), VtValue(xform));

    HdSceneIndexObserver::DirtiedPrimEntries entries;
    entries.push_back({primPath, HdXformSchema::GetDefaultLocator()});

    _SendPrimsDirtied(entries);
}
*/

HdSceneIndexPrim ExecSceneIndex::GetPrim(const SdfPath &primPath) const
{
  HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

  GfMatrix4d matrix = GetXform(primPath);

  prim.dataSource = HdOverlayContainerDataSource::New(
    HdRetainedContainerDataSource::New(
      HdXformSchemaTokens->xform,
      HdXformSchema::Builder()
        .SetMatrix(
            HdRetainedTypedSampledDataSource<GfMatrix4d>::New(matrix))
        .SetResetXformStack(
            HdRetainedTypedSampledDataSource<bool>::New(false))
        .Build()),
    prim.dataSource);

  return prim;
}

SdfPathVector ExecSceneIndex::GetChildPrimPaths(
    const SdfPath &primPath) const
{
  return _GetInputSceneIndex()->GetChildPrimPaths(primPath);
}

void ExecSceneIndex::_PrimsAdded(
    const HdSceneIndexBase &sender,
    const HdSceneIndexObserver::AddedPrimEntries &entries)
{
  _SendPrimsAdded(entries);
}

void ExecSceneIndex::_PrimsRemoved(
    const HdSceneIndexBase &sender,
    const HdSceneIndexObserver::RemovedPrimEntries &entries)
{
  _SendPrimsRemoved(entries);
}
void ExecSceneIndex::_PrimsDirtied(
    const HdSceneIndexBase &sender,
    const HdSceneIndexObserver::DirtiedPrimEntries &entries)
{
  _SendPrimsDirtied(entries);
}

JVR_NAMESPACE_CLOSE_SCOPE