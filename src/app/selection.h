#ifndef JVR_APP_SELECTION_H
#define JVR_APP_SELECTION_H

#include "../common.h"
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/base/tf/token.h>
#include <pxr/imaging/hd/selection.h>
#include <vector>

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
    SdfPath             path;
    std::vector<int>    components;
    std::vector<float>  weights;
    size_t              hash;

    void ComputeHash();
  };

  void ComputeHash();
  size_t GetHash() { return _hash; };
  void AddItem(const SdfPath& path);
  void RemoveItem(const SdfPath& item);
  void ToggleItem(const SdfPath& item);

  void AddComponent(const SdfPath& object,
    Type type, int index);
  void RemoveComponent(const SdfPath& object,
    Type type, int index);
  void AddComponents(const SdfPath& object,
    Type type, std::vector<int> indices);
  void RemoveComponents(const SdfPath& object,
    Type type, std::vector<int> indices);
  void Clear();

  bool IsEmpty() const;
  bool IsObject() const;
  bool IsComponent() const;
  bool IsAttribute() const;

  void SetMode(Mode mode) {_mode = mode; };
  Mode GetMode() { return _mode; };

  bool IsPickablePath(const UsdStage& stage, 
    const SdfPath& path);
  SdfPathVector GetSelectedPaths() const;
  SdfPath GetAnchorPath() const;

  size_t GetNumSelectedItems() { return _items.size(); };
  Item& operator[](size_t index) {
    return _items[index];
  };
  Item& GetItem(size_t index) {
    return _items[index];
  };
  std::vector<Item>& GetItems() { return _items; };
  void SetItems(const std::vector<Item>& items) {_items = items;};

  bool IsSelected(const UsdPrim& prim) const;
  bool IsSelected(const SdfPrimSpec& prim) const;

private:
  bool                        _CheckKind(Mode mode, const TfToken& kind);
  Mode                        _mode = Mode::MODEL;
  std::vector<Item>           _items;
  size_t                      _hash;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APP_SELECTION_H