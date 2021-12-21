#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include "../app/selection.h"
#include "../app/application.h"
#include "../app/notice.h"

JVR_NAMESPACE_OPEN_SCOPE

bool Selection::IsEmpty()
{
  return _items.size() == 0;
}

bool Selection::IsObject()
{
  for (auto& item : _items) {
    if (item.type == Type::OBJECT) return true;
  }
  return false;
}

bool Selection::IsComponent()
{
  for (auto& item : _items) {
    if (item.type == Type::VERTEX ||
        item.type == Type::EDGE ||
        item.type == Type::FACE) return true;
  }
  return false;
}

bool Selection::IsAttribute()
{
  for (auto& item : _items) {
    if (item.type == Type::ATTRIBUTE) return true;
  }
  return false;
}

void Selection::AddItem(const pxr::SdfPath& path)
{
  for (auto& item: _items) {
    if (path == item.path)return;
  }
  _items.push_back({ Type::OBJECT, path });
  SelectionChangedNotice().Send();
}

void Selection::RemoveItem(const pxr::SdfPath& path)
{
  for (auto it = _items.begin(); it < _items.end(); ++it) {
    if (path == it->path) _items.erase(it);
  }
  SelectionChangedNotice().Send();
}

void Selection::ToggleItem(const pxr::SdfPath& path)
{
  for (auto it = _items.begin(); it < _items.end(); ++it) {
    if (path == it->path) {
      _items.erase(it);
      return;
    }
  }
  AddItem(path);
}

void Selection::AddComponent(const pxr::SdfPath& object,
  Type type, int index)
{

}

void Selection::RemoveComponent(const pxr::SdfPath& object,
  Type type, int index)
{

}

void Selection::AddComponents(const pxr::SdfPath& object,
  Type type, std::vector<int> indices)
{

}

void Selection::RemoveComponents(const pxr::SdfPath& object,
  Type type, std::vector<int> indices)
{

}

void Selection::Clear()
{
  _items.clear();
}

pxr::SdfPathVector Selection::GetSelectedPrims()
{
  pxr::SdfPathVector selectedPrims;
  for (const auto& item : _items) {
    selectedPrims.push_back(item.path);
  }
  return selectedPrims;
}


JVR_NAMESPACE_CLOSE_SCOPE