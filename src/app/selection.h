#ifndef JVR_APP_SELECTION_H
#define JVR_APP_SELECTION_H

#include "../common.h"
#include <pxr/usd/sdf/path.h>
#include <pxr/base/tf/token.h>
#include <pxr/imaging/hd/selection.h>
#include <vector>
#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>

JVR_NAMESPACE_OPEN_SCOPE


class Selection {
public:
  enum Type {
    PRIM,
    VERTEX,
    EDGE,
    FACE,
    ATTRIBUTE
  };

  enum Mode {
    ASSEMBLY,
    MODEL,
    GROUP,
    COMPONENT,
    SUBCOMPONENT
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

  void SetMode(Mode mode) { std::cout << "set mode : " << mode << std::endl; _mode = mode; };
  Mode GetMode() { return _mode; };

  bool IsPickablePath(const pxr::UsdStage& stage, 
    const pxr::SdfPath& path);
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

  bool IsSelected(const pxr::UsdPrim& prim);

private:
  bool                        _CheckKind(Mode mode, const pxr::TfToken& kind);
  Mode                        _mode = Mode::MODEL;
  std::vector<Item>           _items;
  size_t                      _hash;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APP_SELECTION_H