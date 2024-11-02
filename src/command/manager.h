#ifndef JVR_COMMAND_MANAGER_H
#define JVR_COMMAND_MANAGER_H

#include <stack>
#include <iostream>
#include <memory>
#include <deque>
#include <stack>
#include "../common.h"
#include "../command/command.h"
#include "../command/inverse.h"


JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Command Manager
//==================================================================================
typedef std::deque<std::shared_ptr<Command>> CommandQueue_t;
typedef std::stack<std::shared_ptr<Command>> CommandStack_t;

class CommandManager {
public:
  CommandManager() {};
  void AddCommand(std::shared_ptr<Command> command);
  void AddDeferredCommand(std::shared_ptr<Command> command);
  void ExecuteCommands();
  void ExecuteDeferredCommands();
  void Undo();
  void Redo();
  void Clear();

  // singleton 
  static CommandManager *Get();

private:
  static CommandManager*      _singleton;
  CommandQueue_t              _todoStack;
  CommandQueue_t              _undoStack;
  CommandQueue_t              _redoStack;
  CommandStack_t              _deferredStack;
  std::vector<UndoInverse>    _inverse;
};

#define ADD_COMMAND(CMD, ...) \
CommandManager::Get()->AddCommand(std::shared_ptr<CMD>( new CMD(__VA_ARGS__)));

#define ADD_DEFERRED_COMMAND(CMD, ...) \
CommandManager::Get()->AddDeferredCommand(std::shared_ptr<CMD>( new CMD(__VA_ARGS__)));

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_COMMAND_MANAGER_H