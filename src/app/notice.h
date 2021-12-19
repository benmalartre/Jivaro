#ifndef JVR_APPLICATION_NOTICE_H
#define JVR_APPLICATION_NOTICE_H

#include "../common.h"
#include <pxr/base/tf/notice.h>
#include <pxr/base/tf/type.h>

JVR_NAMESPACE_OPEN_SCOPE


/// Notifications sent by the Jivaro Application

/// Base class for all Plug notices.
class BaseNotice : public pxr::TfNotice
{
public:
  BaseNotice();
  virtual ~BaseNotice();
};

/// Notice sent after new scene
class NewSceneNotice : public BaseNotice
{
public:
  explicit NewSceneNotice();
  virtual ~NewSceneNotice();


private:
};

/// Notice sent after selection changed
class SelectionChangedNotice : public BaseNotice
{
public:
  explicit SelectionChangedNotice();
  virtual ~SelectionChangedNotice();


private:
};

/// Notice sent after scene changed
class SceneChangedNotice : public BaseNotice
{
public:
  explicit SceneChangedNotice();
  virtual ~SceneChangedNotice();


private:
};

JVR_NAMESPACE_CLOSE_SCOPE
#endif //JVR_APPLICATION_NOTICE_H