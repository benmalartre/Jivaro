#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/kind/registry.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include "../app/selection.h"
#include "../app/application.h"
#include "../app/notice.h"

JVR_NAMESPACE_OPEN_SCOPE

void 
Selection::Item::ComputeHash()
{
  hash = TfHash::Combine(
    type,
    path,
    components,
    weights);
}

void 
Selection::ComputeHash()
{
  _hash = 0;
  for (auto& item: _items) {
   _hash = TfHash::Combine(_hash,item.hash);
  }
}

bool 
Selection::IsSelected(const UsdPrim& prim) const
{
  const SdfPath primPath = prim.GetPath();
  for (auto& item : _items) {
    if (item.path == primPath) return true;
  }
  return false;
}

bool 
Selection::IsSelected(const SdfPrimSpec& prim) const
{
  const SdfPath primPath = prim.GetPath();
  for (auto& item : _items) {
    if (item.path == primPath) return true;
  }
  return false;
}

bool 
Selection::IsEmpty() const
{
  return _items.size() == 0;
}

bool 
Selection::IsObject() const
{
  for (auto& item : _items) {
    if (item.type != Type::PRIM) return false;
  }
  return true;
}

bool 
Selection::IsComponent() const
{
  for (auto& item : _items) {
    if (!(item.type == Type::VERTEX ||
      item.type == Type::EDGE ||
      item.type == Type::FACE)) return false;
  }
  return true;
}

bool 
Selection::IsAttribute() const
{
  for (auto& item : _items) {
    if (item.type != Type::ATTRIBUTE) return false;
  }
  return true;
}

bool 
Selection::IsPickablePath(const UsdStage& stage,
  const SdfPath& path) 
{
  auto prim = stage.GetPrimAtPath(path);
  if (prim.IsPseudoRoot())
    return true;

  if (_mode == Selection::PRIM)
    return true;

  TfToken primKind;
  UsdModelAPI(prim).GetKind(&primKind);
  if (_mode == Selection::MODEL && KindRegistry::GetInstance().IsA(primKind, KindTokens->model)) {
    return true;
  }
  if (_mode == Selection::ASSEMBLY &&
    KindRegistry::GetInstance().IsA(primKind, KindTokens->assembly)) {
    return true;
  }

    // Other possible tokens
    // KindTokens->component
    // KindTokens->group
    // KindTokens->subcomponent

    // We can also test for xformable or other schema API
  return false;
}

void 
Selection::AddItem(const SdfPath& path)
{
  for (auto& item: _items) {
    if (path == item.path)return;
  }
  _items.push_back({ Type::PRIM, path });
  _items.back().ComputeHash();
  ComputeHash();

}

void 
Selection::RemoveItem(const SdfPath& path)
{
  for (auto it = _items.begin(); it < _items.end(); ++it) {
    if (path == it->path) _items.erase(it);
  }
  ComputeHash();
}

void 
Selection::ToggleItem(const SdfPath& path)
{
  for (auto it = _items.begin(); it < _items.end(); ++it) {
    if (path == it->path) {
      _items.erase(it);
      ComputeHash();
      return;
    }
  }
  AddItem(path);
  ComputeHash();
}

void 
Selection::AddComponent(const SdfPath& object,
  Type type, int index)
{
  ComputeHash();
}

void 
Selection::RemoveComponent(const SdfPath& object,
  Type type, int index)
{
  ComputeHash();
}

void 
Selection::AddComponents(const SdfPath& object,
  Type type, std::vector<int> indices)
{
  ComputeHash();
}

void 
Selection::RemoveComponents(const SdfPath& object,
  Type type, std::vector<int> indices)
{
  ComputeHash();
}

void 
Selection::Clear()
{
  _items.clear();
  ComputeHash();
}

SdfPath
Selection::GetAnchorPath() const
{
  SdfPathVector paths = GetSelectedPaths();
  if(!paths.size()) return SdfPath::AbsoluteRootPath();
  return paths[0];
}



void _RecurseAffectedPrims(const UsdPrim& prim, SdfPathVector& results)
{
  results.push_back(prim.GetPath());
  for(const UsdPrim child: prim.GetChildren()) {
    _RecurseAffectedPrims(child, results);
  }
}

SdfPathVector 
Selection::ComputeAffectedPaths(UsdStageRefPtr& stage)
{
  SdfPathVector results;
  for(auto& item: _items) {
    if(item.type == Selection::PRIM) {
      UsdPrim prim = stage->GetPrimAtPath(item.path);
      if(prim.IsValid())
        _RecurseAffectedPrims(prim, results);
    }
  }
  return results;
}

SdfPathVector 
Selection::GetSelectedPaths() const
{
  SdfPathVector selectedPaths;
  for (const auto& item : _items) {
    selectedPaths.push_back(item.path);
  }
  return selectedPaths;
}


JVR_NAMESPACE_CLOSE_SCOPE