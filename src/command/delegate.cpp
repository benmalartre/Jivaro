
#include <functional>

#include "../command/delegate.h"
#include "../command/router.h"

PXR_NAMESPACE_OPEN_SCOPE

UndoStateDelegate::UndoStateDelegate()
  : _dirty(false) {
  //TfDebug::Enable(USDQT_DEBUG_UNDOSTATEDELEGATE);
}

UndoStateDelegateRefPtr UndoStateDelegate::New() {
  return pxr::TfCreateRefPtr(new UndoStateDelegate());
}

bool UndoStateDelegate::_IsDirty() { return _dirty; }

void UndoStateDelegate::_MarkCurrentStateAsClean() {
  _dirty = false;
}

void UndoStateDelegate::_MarkCurrentStateAsDirty() { _dirty = true; }

void UndoStateDelegate::_OnSetLayer(const SdfLayerHandle& layer) {
  if (layer)
    _layer = layer;
  else {
    _layer = NULL;
    // is this an error?
  }
}

void UndoStateDelegate::_RouteInverse(
  std::function<bool()> inverse) {
  if (!UndoRouter::IsMuted())
    UndoRouter::Get().AddInverse(inverse);
  else {
    TF_WARN("Performance Warning.  Inverse should be muted earlier in stack.");
  }
}

bool UndoStateDelegate::_InvertSetField(const SdfPath& path,
  const TfToken& fieldName,
  const VtValue& inverse) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting Field '%s' for Spec '%s'\n", fieldName.GetText(),
      path.GetText());*/
  if (!_layer) {
    TF_CODING_ERROR("Cannot invert field for expired layer.");
    return false;
  }
  SetField(path, fieldName, inverse);
  return true;
}

bool UndoStateDelegate::_InvertSetFieldDictValueByKey(
  const SdfPath& path, const TfToken& fieldName, const TfToken& keyPath,
  const VtValue& inverse) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting Field '%s' By Key '%s' for Spec '%s'\n",
      fieldName.GetText(), keyPath.GetText(), path.GetText());*/
  if (!_layer) {
    TF_CODING_ERROR(
      "Cannot invert field dictionary value for expired layer.");
    return false;
  }
  SetFieldDictValueByKey(path, fieldName, keyPath,
    inverse);
  return true;
}

bool UndoStateDelegate::_InvertSetTimeSample(
  const SdfPath& path, double time, const VtValue& inverse) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting TimeSample '%f' for Spec '%s'\n", time, path.GetText());*/
  if (!_layer) {
    TF_CODING_ERROR("Cannot invert time sample for expired layer.");
    return false;
  }
  SetTimeSample(path, time, inverse);
  return true;
}

bool UndoStateDelegate::_InvertCreateSpec(const SdfPath& path,
  bool inert) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting Creation of Spec '%s'\n", path.GetText());*/
  if (!_layer) {
    TF_CODING_ERROR("Cannot invert spec creation for expired layer.");
    return false;
  }
  DeleteSpec(path, inert);
  return true;
}

/// XXX: This is copied straight from Sd.  Should this be refactored and
/// packaged as a part of Sdf?
static void _CopySpec(const SdfAbstractData& src, SdfAbstractData* dst,
  const SdfPath& path) {
  dst->CreateSpec(path, src.GetSpecType(path));

  std::vector<TfToken> fields = src.List(path);
  TF_FOR_ALL(i, fields) { dst->Set(path, *i, src.Get(path, *i)); }
}

bool UndoStateDelegate::_InvertDeleteSpec(
  const SdfPath& path, bool inert, SdfSpecType deletedSpecType,
  const SdfDataRefPtr& deletedData) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting Deletion of Spec '%s'\n", path.GetText());*/
  if (!_layer) {
    TF_CODING_ERROR("Cannot invert spec deletion for expired layer.");
    return false;
  }
  SdfChangeBlock changeBlock;

  CreateSpec(path, deletedSpecType, inert);

  struct _SpecCopier : public SdfAbstractDataSpecVisitor {
    explicit _SpecCopier(SdfAbstractData* dst_) : dst(dst_) {}

    virtual bool VisitSpec(const SdfAbstractData& src,
      const SdfPath& path) {
      _CopySpec(src, dst, path);
      return true;
    }

    virtual void Done(const SdfAbstractData&) {
      // Do nothing
    }

    SdfAbstractData* const dst;
  };

  _SpecCopier specCopier(boost::get_pointer(_GetLayerData()));
  deletedData->VisitSpecs(&specCopier);
  return true;
}

bool UndoStateDelegate::_InvertMoveSpec(const SdfPath& oldPath,
  const SdfPath& newPath) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting move of '%s' to '%s'\n", oldPath.GetText(),
      newPath.GetText());*/
  if (!_layer) {
    TF_CODING_ERROR("Cannot invert spec move for expired layer.");
    return false;
  }
  MoveSpec(newPath, oldPath);
  return true;
}

bool UndoStateDelegate::_InvertPushTokenChild(
  const SdfPath& parentPath, const TfToken& fieldName, const TfToken& value) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting push field '%s' of '%s'\n", fieldName.GetText(),
      value.GetText());*/

  if (!_layer) {
    TF_CODING_ERROR("Cannot invert push child for expired layer.");
    return false;
  }
  PopChild(parentPath, fieldName, value);
  return true;
}
bool UndoStateDelegate::_InvertPopTokenChild(
  const SdfPath& parentPath, const TfToken& fieldName, const TfToken& value) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting pop field '%s' of '%s'\n", fieldName.GetText(),
      value.GetText());*/
  if (!_layer) {
    TF_CODING_ERROR("Cannot invert pop child for expired layer.");
    return false;
  }
  PushChild(parentPath, fieldName, value);

  return true;
}
bool UndoStateDelegate::_InvertPushPathChild(
  const SdfPath& parentPath, const TfToken& fieldName, const SdfPath& value) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting push field '%s' of '%s'\n", fieldName.GetText(),
      value.GetText());*/
  if (!_layer) {
    TF_CODING_ERROR("Cannot invert push child for expired layer.");
    return false;
  }
  PopChild(parentPath, fieldName, value);
  return true;
}
bool UndoStateDelegate::_InvertPopPathChild(
  const SdfPath& parentPath, const TfToken& fieldName, const SdfPath& value) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Inverting pop field '%s' of '%s'\n", fieldName.GetText(),
      value.GetText());*/
  if (!_layer) {
    TF_CODING_ERROR("Cannot invert pop child for expired layer.");
    return false;
  }
  PushChild(parentPath, fieldName, value);
  return true;
}

void UndoStateDelegate::_OnSetFieldImpl(
  const SdfPath& path, const TfToken& fieldName) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Setting Field '%s' for Spec '%s'\n", fieldName.GetText(),
      path.GetText());*/
  _MarkCurrentStateAsDirty();
  const VtValue inverseValue = _layer->GetField(path, fieldName);
  _RouteInverse(std::bind(&UndoStateDelegate::_InvertSetField,
    this, path, fieldName,
    inverseValue));
}

void UndoStateDelegate::_OnSetFieldDictValueByKeyImpl(
  const SdfPath& path, const TfToken& fieldName,
  const TfToken& keyPath) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Setting Dictionary Field '%s' By Key '%s' for Spec '%s'\n",
      fieldName.GetText(), keyPath.GetText(),
      path.GetText());*/
  _MarkCurrentStateAsDirty();
  const VtValue inverseValue =
    _layer->GetFieldDictValueByKey(path, fieldName, keyPath);
  _RouteInverse(std::bind(
    &UndoStateDelegate::_InvertSetFieldDictValueByKey, this,
    path, fieldName, keyPath, inverseValue));
}

void UndoStateDelegate::_OnSetTimeSampleImpl(
  const SdfPath& path, double time) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Setting Time Sample '%f' for Spec '%s'\n", time,
      path.GetText());*/
  _MarkCurrentStateAsDirty();

  if (!_GetLayer()->HasField(path, SdfFieldKeys->TimeSamples)) {
    _RouteInverse(std::bind(&UndoStateDelegate::_InvertSetField,
      this, path,
      SdfFieldKeys->TimeSamples, VtValue()));
  }
  else {
    VtValue oldValue;
    _GetLayer()->QueryTimeSample(path, time, &oldValue);
    _RouteInverse(
      std::bind(&UndoStateDelegate::_InvertSetTimeSample, this,
        path, time, oldValue));
  }
}

void UndoStateDelegate::_OnSetField(const SdfPath& path,
  const TfToken& fieldName,
  const VtValue& value) {
  _OnSetFieldImpl(path, fieldName);
}

void UndoStateDelegate::_OnSetField(
  const SdfPath& path, const TfToken& fieldName,
  const SdfAbstractDataConstValue& value) {
  _OnSetFieldImpl(path, fieldName);
}

void UndoStateDelegate::_OnSetFieldDictValueByKey(
  const SdfPath& path, const TfToken& fieldName,
  const TfToken& keyPath, const VtValue& value) {
  _OnSetFieldDictValueByKeyImpl(path, fieldName, keyPath);
}

void UndoStateDelegate::_OnSetFieldDictValueByKey(
  const SdfPath& path, const TfToken& fieldName,
  const TfToken& keyPath, const SdfAbstractDataConstValue& value) {
  _OnSetFieldDictValueByKeyImpl(path, fieldName, keyPath);
}

void UndoStateDelegate::_OnSetTimeSample(
  const SdfPath& path, double time, const VtValue& value) {
  _OnSetTimeSampleImpl(path, time);
}

void UndoStateDelegate::_OnSetTimeSample(
  const SdfPath& path, double time,
  const SdfAbstractDataConstValue& value) {
  _OnSetTimeSampleImpl(path, time);
}

void UndoStateDelegate::_OnCreateSpec(const SdfPath& path,
  SdfSpecType specType,
  bool inert) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Creating spec at '%s'\n", path.GetText());*/
  _MarkCurrentStateAsDirty();
  _RouteInverse(std::bind(&UndoStateDelegate::_InvertCreateSpec,
    this, path, inert));
}

// XXX: This is copied from SdLayer
static void _CopySpecAtPath(const SdfAbstractData& src, SdfAbstractData* dst,
  const SdfPath& path) {
  _CopySpec(src, dst, path);
}

void UndoStateDelegate::_OnDeleteSpec(const SdfPath& path,
  bool inert) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Deleting spec at '%s'\n", path.GetText());*/
  _MarkCurrentStateAsDirty();

  SdfDataRefPtr deletedData = TfCreateRefPtr(new SdfData());
  SdfLayer::TraversalFunction copyFunc = std::bind(
    &_CopySpecAtPath,
    boost::cref(*boost::get_pointer(_GetLayerData())),
    boost::get_pointer(deletedData), std::placeholders::_1);
  _GetLayer()->Traverse(path, copyFunc);

  const SdfSpecType deletedSpecType = _GetLayer()->GetSpecType(path);

  _RouteInverse(std::bind(&UndoStateDelegate::_InvertDeleteSpec,
    this, path, inert, deletedSpecType, deletedData));
}

void UndoStateDelegate::_OnMoveSpec(const SdfPath& oldPath,
  const SdfPath& newPath) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Moving spec at '%s' to '%s'\n", oldPath.GetText(),
      newPath.GetText());*/
  _MarkCurrentStateAsDirty();

  _RouteInverse(std::bind(&UndoStateDelegate::_InvertMoveSpec,
    this, oldPath, newPath));
}
void UndoStateDelegate::_OnPushChild(const SdfPath& parentPath,
  const TfToken& fieldName,
  const TfToken& value) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Pushing field '%s' of '%s'\n", fieldName.GetText(),
      parentPath.GetText());*/
  _MarkCurrentStateAsDirty();

  _RouteInverse(
    std::bind(&UndoStateDelegate::_InvertPushTokenChild, this,
      parentPath, fieldName, value));
}
void UndoStateDelegate::_OnPushChild(const SdfPath& parentPath,
  const TfToken& fieldName,
  const SdfPath& value) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Pushing field '%s' of '%s'\n", fieldName.GetText(),
      parentPath.GetText());*/
  _MarkCurrentStateAsDirty();

  _RouteInverse(std::bind(&UndoStateDelegate::_InvertPushPathChild,
    this, parentPath, fieldName, value));
}

void UndoStateDelegate::_OnPopChild(const SdfPath& parentPath,
  const TfToken& fieldName,
  const TfToken& oldValue) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Popping field '%s' of '%s'\n", fieldName.GetText(),
      parentPath.GetText());*/
  _MarkCurrentStateAsDirty();

  _RouteInverse(std::bind(&UndoStateDelegate::_InvertPopTokenChild,
    this, parentPath, fieldName, oldValue));
}

void UndoStateDelegate::_OnPopChild(const SdfPath& parentPath,
  const TfToken& fieldName,
  const SdfPath& oldValue) {
  /*
  TF_DEBUG(USDQT_DEBUG_UNDOSTATEDELEGATE)
    .Msg("Popping field '%s' of '%s'\n", fieldName.GetText(),
      parentPath.GetText());*/
  _MarkCurrentStateAsDirty();

  _RouteInverse(std::bind(&UndoStateDelegate::_InvertPopPathChild,
    this, parentPath, fieldName, oldValue));
}

PXR_NAMESPACE_CLOSE_SCOPE