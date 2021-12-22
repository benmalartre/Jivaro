#ifndef JVR_COMMAND_DELEGATE_H
#define JVR_COMMAND_DELEGATE_H

#include "../common.h"
#include <pxr/base/tf/declarePtrs.h>
#include "pxr/usd/sdf/api.h"
#include "pxr/usd/sdf/declareHandles.h"
#include "pxr/usd/sdf/types.h"
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/layerStateDelegate.h>
#include <pxr/usd/sdf/abstractData.h>


PXR_NAMESPACE_OPEN_SCOPE

SDF_DECLARE_HANDLES(SdfLayer);

TF_DECLARE_WEAK_AND_REF_PTRS(LayerStateDelegate);
TF_DECLARE_WEAK_PTRS(SdfAbstractData);

class pxr::SdfAbstractDataConstValue;
class pxr::SdfPath;
class pxr::TfToken;

/// \class SdfSimpleLayerStateDelegate
/// A layer state delegate that simply records whether any changes have
/// been made to a layer.
class LayerStateDelegate
  : public pxr::SdfLayerStateDelegateBase
{
public:
  static LayerStateDelegateRefPtr New();

protected:
  LayerStateDelegate();

  // SdfLayerStateDelegateBase overrides
  virtual bool _IsDirty() override;
  virtual void _MarkCurrentStateAsClean() override;
  virtual void _MarkCurrentStateAsDirty() override;

  virtual void _OnSetLayer(
    const pxr::SdfLayerHandle& layer) override;

  virtual void _OnSetField(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::VtValue& value) override;
  virtual void _OnSetField(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::SdfAbstractDataConstValue& value) override;
  virtual void _OnSetFieldDictValueByKey(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath,
    const pxr::VtValue& value) override;
  virtual void _OnSetFieldDictValueByKey(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath,
    const pxr::SdfAbstractDataConstValue& value) override;

  virtual void _OnSetTimeSample(
    const pxr::SdfPath& path,
    double time,
    const pxr::VtValue& value) override;
  virtual void _OnSetTimeSample(
    const pxr::SdfPath& path,
    double time,
    const pxr::SdfAbstractDataConstValue& value) override;

  virtual void _OnCreateSpec(
    const pxr::SdfPath& path,
    pxr::SdfSpecType specType,
    bool inert) override;

  virtual void _OnDeleteSpec(
    const pxr::SdfPath& path,
    bool inert) override;

  virtual void _OnMoveSpec(
    const pxr::SdfPath& oldPath,
    const pxr::SdfPath& newPath) override;

  virtual void _OnPushChild(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& value) override;
  virtual void _OnPushChild(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::SdfPath& value) override;
  virtual void _OnPopChild(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& oldValue) override;
  virtual void _OnPopChild(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::SdfPath& oldValue) override;

private:
  bool _dirty;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_COMMAND_DELEGATE_H