#include "../command/delegate.h"

PXR_NAMESPACE_OPEN_SCOPE

LayerStateDelegateRefPtr 
LayerStateDelegate::New()
{
    return pxr::TfCreateRefPtr(new LayerStateDelegate);
}

LayerStateDelegate::LayerStateDelegate()
  : _dirty(false)
{
}

bool
LayerStateDelegate::_IsDirty()
{
    return _dirty;
}

void
LayerStateDelegate::_MarkCurrentStateAsClean()
{
    _dirty = false;
}

void
LayerStateDelegate::_MarkCurrentStateAsDirty()
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetLayer(
    const pxr::SdfLayerHandle& layer)
{
}

void 
LayerStateDelegate::_OnSetField(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::VtValue& value)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetField(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::SdfAbstractDataConstValue& value)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetFieldDictValueByKey(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath,
    const pxr::VtValue& value)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetFieldDictValueByKey(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath,
    const pxr::SdfAbstractDataConstValue& value)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetTimeSample(
    const pxr::SdfPath& path,
    double time,
    const pxr::VtValue& value)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetTimeSample(
    const pxr::SdfPath& path,
    double time,
    const pxr::SdfAbstractDataConstValue& value)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnCreateSpec(
    const pxr::SdfPath& path,
    pxr::SdfSpecType specType,
    bool inert)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnDeleteSpec(
    const pxr::SdfPath& path,
    bool inert)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnMoveSpec(
    const pxr::SdfPath& oldPath,
    const pxr::SdfPath& newPath)
{
    _dirty = true;
}

void
LayerStateDelegate::_OnPushChild(
    const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& value)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnPushChild(
    const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::SdfPath& value)
{
    _dirty = true;
}

void
LayerStateDelegate::_OnPopChild(
    const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& oldValue)
{
    _dirty = true;
}

void 
LayerStateDelegate::_OnPopChild(
    const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::SdfPath& oldValue)
{
    _dirty = true;
}

PXR_NAMESPACE_CLOSE_SCOPE