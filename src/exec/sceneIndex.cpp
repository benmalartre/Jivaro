#include <pxr/base/vt/value.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/xformSchema.h>
#include <pxr/imaging/hd/basisCurvesSchema.h>
#include <pxr/imaging/hd/primvarSchema.h>
#include <pxr/imaging/hd/primvarsSchema.h>
#include <pxr/imaging/hd/purposeSchema.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/imaging/hd/visibilitySchema.h>

#include "../exec/sceneIndex.h"

JVR_NAMESPACE_OPEN_SCOPE


ExecSceneIndex::ExecSceneIndex(const HdSceneIndexBaseRefPtr  &inputSceneIndex)
  : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
  _gridPath = SdfPath("/Grid");
  _prim = _CreateGridPrim();
  Populate(true);
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

void 
ExecSceneIndex::Populate(bool populate)
{
  if (populate && !_isPopulated) {
    _SendPrimsAdded({{_gridPath, HdPrimTypeTokens->basisCurves}});
  }
  else if (!populate && _isPopulated) {
    _SendPrimsRemoved({{_gridPath}});
  }
  _isPopulated = populate;
}

HdSceneIndexPrim ExecSceneIndex::GetPrim(const SdfPath &primPath) const
{
  if (primPath == _gridPath) return _prim;
  else return this->GetInputScenes().back()->GetPrim(primPath);
}

SdfPathVector ExecSceneIndex::GetChildPrimPaths(
    const SdfPath &primPath) const
{
  if (!_isPopulated) return {};
    if (primPath == SdfPath::AbsoluteRootPath()) return {_gridPath};
    else return {};
}


HdSceneIndexPrim 
ExecSceneIndex::_CreateGridPrim()
{
  VtVec3fArray colors = {};
  VtVec3fArray pts = {};
  VtIntArray vertexCounts = {};

  GfVec3f mainColor(.0f, .0f, .0f);
  GfVec3f subColor(.3f, .3f, .3f);

  int lines = 10;
  float padding = 1;

  pts.push_back(GfVec3f(0, 0, -padding * lines));
  pts.push_back(GfVec3f(0, 0, padding * lines));
  pts.push_back(GfVec3f(-padding * lines, 0, 0));
  pts.push_back(GfVec3f(padding * lines, 0, 0));

  for (int j = 0; j < 2; j++) {
    vertexCounts.push_back(2);
    colors.push_back(mainColor);
    colors.push_back(mainColor);
  }

  for (int i = 1; i <= lines; i++) {
    pts.push_back(GfVec3f(-padding * i, 0, -padding * lines));
    pts.push_back(GfVec3f(-padding * i, 0, padding * lines));
    pts.push_back(GfVec3f(padding * i, 0, -padding * lines));
    pts.push_back(GfVec3f(padding * i, 0, padding * lines));

    pts.push_back(GfVec3f(-padding * lines, 0, -padding * i));
    pts.push_back(GfVec3f(padding * lines, 0, -padding * i));
    pts.push_back(GfVec3f(-padding * lines, 0, padding * i));
    pts.push_back(GfVec3f(padding * lines, 0, padding * i));

    for (int j = 0; j < 4; j++) {
      vertexCounts.push_back(2);
      colors.push_back(subColor);
      colors.push_back(subColor);
    }
  }

  using _IntArrayDataSource = HdRetainedTypedSampledDataSource<VtIntArray>;
  using _TokenDataSource = HdRetainedTypedSampledDataSource<TfToken>;

  using _PointDataSource = HdRetainedTypedSampledDataSource<VtVec3fArray>;
  using _FloatDataSource = HdRetainedTypedSampledDataSource<VtFloatArray>;

  using _BoolDataSource = HdRetainedTypedSampledDataSource<bool>;

  using _MatrixDataSource = HdRetainedTypedSampledDataSource<GfMatrix4d>;

  HdSceneIndexPrim prim = HdSceneIndexPrim(
    {HdPrimTypeTokens->basisCurves,
      HdRetainedContainerDataSource::New(
        HdBasisCurvesSchemaTokens->basisCurves,
        HdBasisCurvesSchema::Builder()
          .SetTopology(
              HdBasisCurvesTopologySchema::Builder()
                .SetCurveVertexCounts(
                  _IntArrayDataSource::New(vertexCounts))
                .SetBasis(_TokenDataSource::New(HdTokens->bezier))
                .SetType(_TokenDataSource::New(HdTokens->linear))
                .SetWrap(_TokenDataSource::New(HdTokens->nonperiodic))
                .Build())
          .Build(),
        HdPrimvarsSchemaTokens->primvars,
        HdRetainedContainerDataSource::New(
          HdPrimvarsSchemaTokens->points,
          HdPrimvarSchema::Builder()
            .SetPrimvarValue(_PointDataSource::New(pts))
            .SetRole(HdPrimvarSchema::BuildRoleDataSource(
              HdPrimvarSchemaTokens->point))
            .SetInterpolation(
                HdPrimvarSchema::BuildInterpolationDataSource(
                  HdPrimvarSchemaTokens->vertex))
            .Build(),
          HdPrimvarsSchemaTokens->widths,
          HdPrimvarSchema::Builder()
            .SetPrimvarValue(_FloatDataSource::New({1}))
            .SetInterpolation(
              HdPrimvarSchema::BuildInterpolationDataSource(
                HdPrimvarSchemaTokens->constant))
            .Build(),
          HdTokens->displayColor,
          HdPrimvarSchema::Builder()
            .SetPrimvarValue(_PointDataSource::New(colors))
            .SetInterpolation(
              HdPrimvarSchema::BuildInterpolationDataSource(
                HdPrimvarSchemaTokens->vertex))
            .SetRole(HdPrimvarSchema::BuildRoleDataSource(
              HdPrimvarSchemaTokens->color))
            .Build()),
        HdPurposeSchemaTokens->purpose,
        HdPurposeSchema::Builder()
          .SetPurpose(
            _TokenDataSource::New(HdRenderTagTokens->geometry))
          .Build(),
        HdVisibilitySchemaTokens->visibility,
        HdVisibilitySchema::Builder()
          .SetVisibility(_BoolDataSource::New(true))
          .Build(),
        HdXformSchemaTokens->xform,
        HdXformSchema::Builder()
          .SetMatrix(_MatrixDataSource::New(GfMatrix4d(1)))
          .SetResetXformStack(_BoolDataSource::New(false))
          .Build())});
  return prim;
}

JVR_NAMESPACE_CLOSE_SCOPE