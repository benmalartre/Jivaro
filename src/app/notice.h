#ifndef AMN_APPLICATION_NOTICE_H
#define AMN_APPLICATION_NOTICE_H

#include "../common.h"
#include <pxr/base/tf/notice.h>
#include <pxr/base/tf/type.h>
#include <pxr/base/tf/instantiateType.h>

AMN_NAMESPACE_OPEN_SCOPE

class NewSceneNotice : public pxr::TfNotice
{
public:
  NewSceneNotice() {};
private:
};

static void OnNewScene()
{
  NewSceneNotice().Send();
}

class TimeChangedNotice : public pxr::TfNotice
{
public:
  TimeChangedNotice() {};
private:
};

static void OnTimeChanged()
{
  TimeChangedNotice().Send();
}

AMN_NAMESPACE_CLOSE_SCOPE
#endif //AMN_APPLICATION_NOTICE_H