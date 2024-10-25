#ifndef JVR_COMMAND_MANAGER_H
#define JVR_COMMAND_MANAGER_H

#include <stack>
#include <iostream>
#include <memory>
#include <deque>
#include "../common.h"
#include "../command/command.h"
#include "../command/inverse.h"


JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Command Manager
//==================================================================================
typedef std::deque<std::shared_ptr<Command>> CommandStack_t;

class CommandManager {
public:
  CommandManager() {};
  void AddCommand(std::shared_ptr<Command> command);
  void ExecuteCommands();
  void Undo();
  void Redo();
  void Clear();

  // singleton 
  static CommandManager *Get();

private:
  static CommandManager*      _singleton;
  CommandStack_t              _todoStack;
  CommandStack_t              _undoStack;
  CommandStack_t              _redoStack;
  std::vector<UndoInverse>    _inverse;
};

#define ADD_COMMAND(CMD, ...) \
CommandManager::Get()->AddCommand(std::shared_ptr<CMD>( new CMD(__VA_ARGS__)));

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_COMMAND_MANAGER_H