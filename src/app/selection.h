#ifndef JVR_APP_SELECTION_H
#define JVR_APP_SELECTION_H

#include "../common.h"
#include <pxr/usd/sdf/path.h>
#include <pxr/imaging/hd/selection.h>
#include <vector>
#include <boost/optional.hpp>

PXR_NAMESPACE_OPEN_SCOPE


class Selection {
public:
  enum Type {
    OBJECT,
    VERTEX,
    EDGE,
    FACE,
    ATTRIBUTE
  };

  struct Item {
    Type                type;
    pxr::SdfPath        path;
    std::vector<int>    components;
    std::vector<float>  weights;
    size_t              hash;

    void ComputeHash();
  };

  void ComputeHash();
  size_t GetHash() { return _hash; };
	void AddItem(const pxr::SdfPath& item);
  void RemoveItem(const pxr::SdfPath& item);
  void ToggleItem(const pxr::SdfPath& item);

  void AddComponent(const pxr::SdfPath& object,
    Type type, int index);
  void RemoveComponent(const pxr::SdfPath& object,
    Type type, int index);
  void AddComponents(const pxr::SdfPath& object,
    Type type, std::vector<int> indices);
  void RemoveComponents(const pxr::SdfPath& object,
    Type type, std::vector<int> indices);
  void Clear();

  bool IsEmpty();
  bool IsObject();
  bool IsComponent();
  bool IsAttribute();

  pxr::SdfPathVector GetSelectedPrims();
  size_t GetNumSelectedItems() { return _items.size(); };
  Item& operator[](size_t index) {
    return _items[index];
  };
  Item& GetItem(size_t index) {
    return _items[index];
  };
  std::vector<Item>& GetItems() { return _items; };
  void SetItems(const std::vector<Item>& items) {_items = items;};

private:
  Type                        _mode;
  std::vector<Item>           _items;
  uint64_t                    _hash;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APP_SELECTION_H