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
    if (item.type == Type::PRIM) return true;
  }
  return false;
}

bool 
Selection::IsComponent() const
{
  for (auto& item : _items) {
    if (item.type == Type::VERTEX ||
        item.type == Type::EDGE ||
        item.type == Type::FACE) return true;
  }
  return false;
}

bool 
Selection::IsAttribute() const
{
  for (auto& item : _items) {
    if (item.type == Type::ATTRIBUTE) return true;
  }
  return false;
}

bool 
Selection::_CheckKind(Mode mode, const TfToken& kind)
{
  switch (mode) {
  case Mode::COMPONENT:
    return KindRegistry::GetInstance().IsA(kind, KindTokens->model);
  case Mode::GROUP:
    return KindRegistry::GetInstance().IsA(kind, KindTokens->group);
  case Mode::ASSEMBLY:
    return KindRegistry::GetInstance().IsA(kind, KindTokens->assembly);
  case Mode::SUBCOMPONENT:
    return true;
  case Mode::MODEL:
    return KindRegistry::GetInstance().IsA(kind, KindTokens->model);
  }
}

bool 
Selection::IsPickablePath(const UsdStage& stage,
  const SdfPath& path) {
  auto prim = stage.GetPrimAtPath(path);
  if (prim.IsPseudoRoot())
    return true;

  TfToken primKind;
  UsdModelAPI(prim).GetKind(&primKind);
  return(_CheckKind(_mode, primKind));
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