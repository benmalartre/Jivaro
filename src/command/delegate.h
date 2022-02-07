//
// Copyright 2022 benmalartre
//
#ifndef UNDOSTATEDELEGATE_H
#define UNDOSTATEDELEGATE_H

#include "../common.h"

#include <functional>

#include <pxr/pxr.h>
#include <pxr/base/tf/declarePtrs.h>
         
#include <pxr/usd/sdf/layerStateDelegate.h>
#include <pxr/usd/sdf/path.h>
         
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/property.h>

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_WEAK_AND_REF_PTRS(UndoStateDelegate);
TF_DECLARE_WEAK_AND_REF_PTRS(UndoRouter);  // forward declaration

/// \class UndoStateDelegate
///
/// The layer state delegate is a class that forwards the inverse of a given
/// edit to a UndoRouter.  To instantiate this class, create a 
/// UndoRouterPtr, and call yourRouter->TrackLayer(yourLayer).
class UndoStateDelegate : public pxr::SdfLayerStateDelegateBase 
{
private:
  pxr::SdfLayerHandle _layer;
  bool _dirty;

  static UndoStateDelegateRefPtr New();

  UndoStateDelegate();

  void _RouteInverse(std::function<bool()> inverse);

  virtual bool _IsDirty() override;
  virtual void _MarkCurrentStateAsClean() override;
  virtual void _MarkCurrentStateAsDirty() override;

  bool _InvertSetField(const pxr::SdfPath& path, const pxr::TfToken& fieldName,
    const pxr::VtValue& inverse);

  bool _InvertSetFieldDictValueByKey(const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath,
    const pxr::VtValue& inverse);
  bool _InvertSetTimeSample(const pxr::SdfPath& path, double time,
    const pxr::VtValue& inverse);
  bool _InvertCreateSpec(const pxr::SdfPath& path, bool inert);
  bool _InvertDeleteSpec(const pxr::SdfPath& path, bool inert,
    SdfSpecType deletedSpecType,
    const SdfDataRefPtr& deletedData);
  bool _InvertPushTokenChild(const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName, const pxr::TfToken& value);
  bool _InvertPopTokenChild(const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName, const pxr::TfToken& value);
  bool _InvertPushPathChild(const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName, const pxr::SdfPath& value);
  bool _InvertPopPathChild(const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName, const pxr::SdfPath& value);

  bool _InvertMoveSpec(const pxr::SdfPath& oldPath, const pxr::SdfPath& newPath);

  void _OnSetLayer(const SdfLayerHandle& layer) override;

  void _OnSetField(const pxr::SdfPath& path, const pxr::TfToken& fieldName,
    const pxr::VtValue& value) override;
  virtual void _OnSetField(const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::SdfAbstractDataConstValue& value) override;
  void _OnSetFieldImpl(const pxr::SdfPath& path,
    const pxr::TfToken& fieldName);

  virtual void _OnSetFieldDictValueByKey(const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath,
    const pxr::VtValue& value) override;
  virtual void _OnSetFieldDictValueByKey(
    const pxr::SdfPath& path, const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath,
    const pxr::SdfAbstractDataConstValue& value) override;
  void _OnSetFieldDictValueByKeyImpl(const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath);

  virtual void _OnSetTimeSample(const pxr::SdfPath& path, double time,
    const pxr::VtValue& value) override;

  virtual void _OnSetTimeSample(
    const pxr::SdfPath& path, double time,
    const pxr::SdfAbstractDataConstValue& value) override;
  void _OnSetTimeSampleImpl(const pxr::SdfPath& path, double time);

  virtual void _OnCreateSpec(const pxr::SdfPath& path, 
    pxr::SdfSpecType specType, bool inert) override;

  virtual void _OnDeleteSpec(const pxr::SdfPath& path, bool inert) override;

  virtual void _OnMoveSpec(const pxr::SdfPath& oldPath,
    const pxr::SdfPath& newPath) override;
  virtual void _OnPushChild(const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& value) override;
  virtual void _OnPushChild(const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::SdfPath& value) override;

  virtual void _OnPopChild(const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& oldValue) override;

  virtual void _OnPopChild(const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::SdfPath& oldValue) override;

  friend class UndoRouter;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // UNDOSTATEDELEGATE_H