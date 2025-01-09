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

JVR_NAMESPACE_OPEN_SCOPE

PXR_NAMESPACE_USING_DIRECTIVE
TF_DECLARE_WEAK_AND_REF_PTRS(UndoStateDelegate);
TF_DECLARE_WEAK_AND_REF_PTRS(UndoRouter);  // forward declaration

/// \class UndoStateDelegate
///
/// The layer state delegate is a class that forwards the inverse of a given
/// edit to a UndoRouter.  To instantiate this class, create a 
/// UndoRouterPtr, and call yourRouter->TrackLayer(yourLayer).
class UndoStateDelegate : public SdfLayerStateDelegateBase 
{
private:
  SdfLayerHandle _layer;
  bool _dirty;

  static UndoStateDelegateRefPtr New();

  UndoStateDelegate();

  void _RouteInverse(std::function<bool()> inverse);

  virtual bool _IsDirty() override;
  virtual void _MarkCurrentStateAsClean() override;
  virtual void _MarkCurrentStateAsDirty() override;

  bool _InvertSetField(const SdfPath& path, const TfToken& fieldName,
    const VtValue& inverse);

  bool _InvertSetFieldDictValueByKey(const SdfPath& path,
    const TfToken& fieldName,
    const TfToken& keyPath,
    const VtValue& inverse);
  bool _InvertSetTimeSample(const SdfPath& path, double time,
    const VtValue& inverse);
  bool _InvertCreateSpec(const SdfPath& path, bool inert);
  bool _InvertDeleteSpec(const SdfPath& path, bool inert,
    SdfSpecType deletedSpecType,
    const SdfDataRefPtr& deletedData);
  bool _InvertPushTokenChild(const SdfPath& parentPath,
    const TfToken& fieldName, const TfToken& value);
  bool _InvertPopTokenChild(const SdfPath& parentPath,
    const TfToken& fieldName, const TfToken& value);
  bool _InvertPushPathChild(const SdfPath& parentPath,
    const TfToken& fieldName, const SdfPath& value);
  bool _InvertPopPathChild(const SdfPath& parentPath,
    const TfToken& fieldName, const SdfPath& value);

  bool _InvertMoveSpec(const SdfPath& oldPath, const SdfPath& newPath);

  void _OnSetLayer(const SdfLayerHandle& layer) override;

  void _OnSetField(const SdfPath& path, const TfToken& fieldName,
    const VtValue& value) override;
  virtual void _OnSetField(const SdfPath& path,
    const TfToken& fieldName,
    const SdfAbstractDataConstValue& value) override;
  void _OnSetFieldImpl(const SdfPath& path,
    const TfToken& fieldName);

  virtual void _OnSetFieldDictValueByKey(const SdfPath& path,
    const TfToken& fieldName,
    const TfToken& keyPath,
    const VtValue& value) override;
  virtual void _OnSetFieldDictValueByKey(
    const SdfPath& path, const TfToken& fieldName,
    const TfToken& keyPath,
    const SdfAbstractDataConstValue& value) override;
  void _OnSetFieldDictValueByKeyImpl(const SdfPath& path,
    const TfToken& fieldName,
    const TfToken& keyPath);

  virtual void _OnSetTimeSample(const SdfPath& path, double time,
    const VtValue& value) override;

  virtual void _OnSetTimeSample(
    const SdfPath& path, double time,
    const SdfAbstractDataConstValue& value) override;
  void _OnSetTimeSampleImpl(const SdfPath& path, double time);

  virtual void _OnCreateSpec(const SdfPath& path, 
    SdfSpecType specType, bool inert) override;

  virtual void _OnDeleteSpec(const SdfPath& path, bool inert) override;

  virtual void _OnMoveSpec(const SdfPath& oldPath,
    const SdfPath& newPath) override;
  virtual void _OnPushChild(const SdfPath& parentPath,
    const TfToken& fieldName,
    const TfToken& value) override;
  virtual void _OnPushChild(const SdfPath& parentPath,
    const TfToken& fieldName,
    const SdfPath& value) override;

  virtual void _OnPopChild(const SdfPath& parentPath,
    const TfToken& fieldName,
    const TfToken& oldValue) override;

  virtual void _OnPopChild(const SdfPath& parentPath,
    const TfToken& fieldName,
    const SdfPath& oldValue) override;

  friend class UndoRouter;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // UNDOSTATEDELEGATE_H