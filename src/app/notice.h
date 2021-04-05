#ifndef AMN_APPLICATION_NOTICE_H
#define AMN_APPLICATION_NOTICE_H

#include "../common.h"
#include <pxr/base/tf/notice.h>
#include <pxr/base/tf/type.h>

AMN_NAMESPACE_OPEN_SCOPE


/// \class Notice
/// Notifications sent by the Amnesia Application
class Notice
{
public:
  /// Base class for all Plug notices.
  class Base : public pxr::TfNotice
  {
  public:
    virtual ~Base();
  };

  /// Notice sent after new scene
  class NewScene : public Base
  {
  public:
    explicit NewScene();
    virtual ~NewScene();


  private:
  };

  /// Notice sent after selection changed
  class SelectionChanged : public Base
  {
  public:
    explicit SelectionChanged();
    virtual ~SelectionChanged();


  private:
  };
/// </summary>

private:
  Notice();
};

AMN_NAMESPACE_CLOSE_SCOPE
#endif //AMN_APPLICATION_NOTICE_H