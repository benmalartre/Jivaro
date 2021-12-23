#include <pxr/usd/sdf/propertySpec.h>

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
  std::cout << "LAYER DELEGATE : SET LAYER : " << layer->GetDisplayName() << std::endl;
}

void 
LayerStateDelegate::_OnSetField(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::VtValue& value)
{
  std::cout << "LAYER DELEGATE : SET FIELD : " << path << ":" << fieldName << "=" << value << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetField(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::SdfAbstractDataConstValue& value)
{
  std::cout << "LAYER DELEGATE : SET FIELD : " << path << ":" << fieldName << "=ABSTRACT" << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetFieldDictValueByKey(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath,
    const pxr::VtValue& value)
{
   std::cout << "LAYER DELEGATE : SET FIELD DICT VALUE BY KEY: " << path << ":" << fieldName <<
     "," << keyPath << "=" << value << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetFieldDictValueByKey(
    const pxr::SdfPath& path,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& keyPath,
    const pxr::SdfAbstractDataConstValue& value)
{
  std::cout << "LAYER DELEGATE : SET FIELD DICT VALUE BY KEY: " << path << ":" << fieldName <<
    "," << keyPath << "=ABSTRACT" << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetTimeSample(
    const pxr::SdfPath& path,
    double time,
    const pxr::VtValue& value)
{
  std::cout << "LAYER DELEGATE : SET TIME SAMPLE: " << path << ":" << time << "=" << value << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnSetTimeSample(
    const pxr::SdfPath& path,
    double time,
    const pxr::SdfAbstractDataConstValue& value)
{
  pxr::VtValue oldValue;
  pxr::VtValue newValue;

  _GetLayer()->QueryTimeSample(path, time, &oldValue);
  value.GetValue(&newValue);

  std::cout << "OLD VALUE : " << oldValue << std::endl;
  std::cout << "NEW VALUE : " << newValue << std::endl;

  std::cout << "LAYER DELEGATE : SET TIME SAMPLE: " << path << ":" << time << "=ABSTRACT" << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnCreateSpec(
    const pxr::SdfPath& path,
    pxr::SdfSpecType specType,
    bool inert)
{
  std::cout << "LAYER DELEGATE : CREATE SPEC: " << path << ":" << specType << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnDeleteSpec(
    const pxr::SdfPath& path,
    bool inert)
{
  std::cout << "LAYER DELEGATE : DELETE SPEC: " << path << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnMoveSpec(
    const pxr::SdfPath& oldPath,
    const pxr::SdfPath& newPath)
{
  std::cout << "LAYER DELEGATE : MOVE SPEC: " << oldPath << "-->" << newPath << std::endl;
    _dirty = true;
}

void
LayerStateDelegate::_OnPushChild(
    const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& value)
{
  std::cout << "LAYER DELEGATE :PUSH CHILD (TOKEN): " << parentPath << ":" << fieldName << value << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnPushChild(
    const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::SdfPath& value)
{
  std::cout << "LAYER DELEGATE :PUSH CHILD (PATH): " << parentPath << ":" << fieldName << value << std::endl;
    _dirty = true;
}

void
LayerStateDelegate::_OnPopChild(
    const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::TfToken& oldValue)
{
  std::cout << "LAYER DELEGATE :POP CHILD (TOKEN): " << parentPath << ":" << fieldName << oldValue << std::endl;
    _dirty = true;
}

void 
LayerStateDelegate::_OnPopChild(
    const pxr::SdfPath& parentPath,
    const pxr::TfToken& fieldName,
    const pxr::SdfPath& oldValue)
{
  std::cout << "LAYER DELEGATE :POP CHILD (PATH): " << parentPath << ":" << fieldName << oldValue << std::endl;
    _dirty = true;
}

PXR_NAMESPACE_CLOSE_SCOPE