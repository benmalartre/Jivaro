#ifndef JVR_COMMAND_COMMAND_H
#define JVR_COMMAND_COMMAND_H

#include "../common.h"
#include "../command/inverse.h"

JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Base class
//==================================================================================
class Command {
  public:
    Command() : _undoable(false) {};
    Command(bool undoable) : _undoable(undoable) {};
    virtual void Do() = 0;
protected:
  bool        _undoable;
  UndoInverse _inverse;

public:
  friend UndoRouter;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_COMMAND_COMMAND_H