#ifndef AMN_APP_SELECTION_H
#define AMN_APP_SELECTION_H

#pragma once

#include "../common.h"
#include <pxr/usd/sdf/path.h>
#include <vector>

AMN_NAMESPACE_OPEN_SCOPE

enum SelectionType {
  OBJECT,
  COMPONENT,
  ATTRIBUTE
};

enum ComponentType {
  VERTEX,
  EDGE,
  FACE
};

struct SelectionItem {
  SelectionType       type;
  pxr::SdfPath        path;
  ComponentType	      componentType;
  std::vector<int>    components;
  std::vector<float>  weights;
};

class Selection {
public:
	void AddItem(const pxr::SdfPath& item);
  void RemoveItem(const pxr::SdfPath& item);

  void AddComponent(const pxr::SdfPath& object,
    ComponentType compType, int index);
  void RemoveComponent(const pxr::SdfPath& object,
    ComponentType compType, int index);
  void AddComponents(const pxr::SdfPath& object,
    ComponentType compType, std::vector<int> indices);
  void RemoveComponents(const pxr::SdfPath& object,
    ComponentType compType, std::vector<int> indices);
  void Clear();

  size_t GetNumSelectedItems() { return _items.size(); };
  SelectionItem& operator[](size_t index) {
    return _items[index];
  };
  SelectionItem& GetItem(size_t index) {
    return _items[index];
  };
  std::vector<SelectionItem>& Get() { return _items; };

private:
  SelectionType               _selectionMode;
  ComponentType               _componentMode;
  std::vector<SelectionItem>  _items;
};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_APP_SELECTION_H