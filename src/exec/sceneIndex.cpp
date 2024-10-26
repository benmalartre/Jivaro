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
#include "../exec/execution.h"
#include "../geometry/geometry.h"
#include "../geometry/mesh.h"
#include "../geometry/curve.h"
#include "../geometry/points.h"
#include "../geometry/scene.h"

JVR_NAMESPACE_OPEN_SCOPE


ExecSceneIndex::ExecSceneIndex(const HdSceneIndexBaseRefPtr  &inputSceneIndex)
  : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
  , _exec(nullptr)
  , _isPopulated(false)
{
  _gridPath = SdfPath("/Grid");
  _prim = _CreateGridPrim();
  Populate(true);
}

ExecSceneIndex::~ExecSceneIndex()
{
  Populate(false);
}

void ExecSceneIndex::SetExec(Execution* exec)
{
  if(_exec)delete _exec;
  _exec = exec;

  if(!_exec)return;

  Scene* scene = _exec->GetScene();
  for(size_t g = 0; g < scene->GetNumGeometries();++g) {
    Geometry* geometry = scene->GetGeometry(g);
    SdfPath path = geometry->GetPrim().GetPath();
    HdSceneIndexPrim indexPrim;
    switch(geometry->GetType()) {
      case Geometry::MESH:
        std::cout << "exec populate mesh : " << path << std::endl;
        if(path.IsEmpty()) continue;
        indexPrim = _GetInputSceneIndex()->GetPrim(path);
        _SendPrimsAdded({{path, HdPrimTypeTokens->mesh}});
        break;

      case Geometry::POINT:
        std::cout << "exec populate points : " << path << std::endl;
        break;

      case Geometry::CURVE:
        std::cout << "exec populate curve : " << path << std::endl;
        break;
    }
  }
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
  if(_exec) {    
    Scene::_Prim* prim = _exec->GetScene()->GetPrim(primPath);
    if(prim) std::cout << primPath << " is exec object : update it !!";
  }

  return _GetInputSceneIndex()->GetPrim(primPath);
}

SdfPathVector ExecSceneIndex::GetChildPrimPaths(
    const SdfPath &primPath) const
{
  if (!_isPopulated) return {};
    if (primPath == SdfPath::AbsoluteRootPath()) return {_gridPath};
    else return {};
}

void ExecSceneIndex::_PrimsAdded(
  const pxr::HdSceneIndexBase &sender,
  const pxr::HdSceneIndexObserver::AddedPrimEntries &entries)
{
  _SendPrimsAdded(entries);
}

void ExecSceneIndex::_PrimsRemoved(
  const pxr::HdSceneIndexBase &sender,
  const pxr::HdSceneIndexObserver::RemovedPrimEntries &entries)
{
  _SendPrimsRemoved(entries);
}

void ExecSceneIndex::_PrimsDirtied(
  const pxr::HdSceneIndexBase &sender,
  const pxr::HdSceneIndexObserver::DirtiedPrimEntries &entries)
{
  _SendPrimsDirtied(entries);
}

void
ExecSceneIndex::_SystemMessage(
  const TfToken &messageType,
  const HdDataSourceBaseHandle &args)
{

  Scene* scene = _exec->GetScene();
  HdSceneIndexObserver::DirtiedPrimEntries dirtied;
  for (auto &prim: scene->GetPrims()) {
    const SdfPath &path = prim.first;
    HdDirtyBits bits = prim.second.bits;

    if(bits != HdChangeTracker::Clean)
      std::cout << "EXEC : dirty " << path << std::endl;
      //dirtied.push_back({ path, HdDataSourceLocator(HdPrimvarsSchemaTokens->points) });
  }

  if (!dirtied.empty()) {
      _SendPrimsDirtied(dirtied);
  }
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