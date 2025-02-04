#ifndef JVR_APPLICATION_NOTICE_H
#define JVR_APPLICATION_NOTICE_H

#include "../common.h"
#include <pxr/base/tf/notice.h>
#include <pxr/base/tf/type.h>

JVR_NAMESPACE_OPEN_SCOPE


/// Notifications sent by the Jivaro Application

/// Base class for all Plug notices.
class BaseNotice : public TfNotice
{
public:
  BaseNotice();
  virtual ~BaseNotice();
};

/// Notice sent after new scene
class UndoStackNotice : public BaseNotice
{
public:
  explicit UndoStackNotice();
  virtual ~UndoStackNotice();
};

/// Notice sent after new scene
class NewSceneNotice : public BaseNotice
{
public:
  explicit NewSceneNotice();
  virtual ~NewSceneNotice();
};

/// Notice sent after selection changed
class SelectionChangedNotice : public BaseNotice
{
public:
  explicit SelectionChangedNotice();
  virtual ~SelectionChangedNotice();
};

/// Notice sent after scene changed
class SceneChangedNotice : public BaseNotice
{
public:
  explicit SceneChangedNotice();
  virtual ~SceneChangedNotice();
};

/// Notice sent after attribute changed
class AttributeChangedNotice : public BaseNotice
{
public:
  explicit AttributeChangedNotice();
  virtual ~AttributeChangedNotice();
};

/// Notice sent after time changed
class TimeChangedNotice : public BaseNotice
{
public:
  explicit TimeChangedNotice();
  virtual ~TimeChangedNotice();
};


/// Notice sent after time changed
class ToolChangedNotice : public BaseNotice
{
public:
  explicit ToolChangedNotice(short tool);
  virtual ~ToolChangedNotice();
  short GetTool() const {return _tool;};
private:
  short _tool;
};


JVR_NAMESPACE_CLOSE_SCOPE
#endif //JVR_APPLICATION_NOTICE_H