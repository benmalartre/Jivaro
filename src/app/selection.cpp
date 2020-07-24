#include "selection.h"


AMN_NAMESPACE_OPEN_SCOPE

void Selection::AddItem(const pxr::SdfPath& path)
{
  for (auto& item: _items) {
    if (path == item.path)return;
  }
	_items.push_back({ SelectionType::OBJECT, path });
}

void Selection::RemoveItem(const pxr::SdfPath& path)
{
  for (auto& it = _items.begin(); it < _items.end(); ++it) {
    if (path == it->path) _items.erase(it);
  }
}
	
void Selection::AddComponent(const pxr::SdfPath& object,
	ComponentType compType, int index)
{
	
}

void Selection::RemoveComponent(const pxr::SdfPath& object,
	ComponentType compType, int index)
{
	
}

void Selection::AddComponents(const pxr::SdfPath& object,
	ComponentType compType, std::vector<int> indices)
{
	
}
	
void Selection::RemoveComponents(const pxr::SdfPath& object,
		ComponentType compType, std::vector<int> indices)
{
	
}

void Selection::Clear()
{
  _items.clear();
}

AMN_NAMESPACE_CLOSE_SCOPE