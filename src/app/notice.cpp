
#include "pxr/pxr.h"
#include "pxr/base/tf/type.h"
#include "pxr/base/tf/registryManager.h"
#include "notice.h"

PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define< AMN::Notice::Base,
    TfType::Bases<TfNotice> >();
  TfType::Define< AMN::Notice::NewScene,
    TfType::Bases<AMN::Notice::Base> >();
  TfType::Define< AMN::Notice::SelectionChanged,
    TfType::Bases<AMN::Notice::Base> >();
}


AMN_NAMESPACE_OPEN_SCOPE

////////////////////////////////////////////////////////////

Notice::Base::~Base()
{
}

////////////////////////////////////////////////////////////

Notice::
NewScene::NewScene()
{
}

Notice::
NewScene::~NewScene()
{
}

////////////////////////////////////////////////////////////

Notice::
SelectionChanged::SelectionChanged()
{
}

Notice::
SelectionChanged::~SelectionChanged()
{
}

AMN_NAMESPACE_CLOSE_SCOPE