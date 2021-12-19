#ifndef JVR_COMMAND_MANAGER_H
#define JVR_COMMAND_MANAGER_H

#include <stack>
#include <iostream>
#include <memory>
#include <deque>
#include "../common.h"
#include "../command/command.h"
#include <thread>
#include <mutex>


JVR_NAMESPACE_OPEN_SCOPE

//==================================================================================
// Command Manager
//==================================================================================
typedef std::deque<std::shared_ptr<Command>> CommandStack_t;

class CommandManager {
  std::mutex     _mutex;
  std::thread    _thread;
  CommandStack_t _todoStack;
  CommandStack_t _undoStack;
  CommandStack_t _redoStack;

public:
  CommandManager() {}
  void Start();
  void AddCommand(std::shared_ptr<Command> command);
  void ExecuteCommands();
  void Undo();
  void Redo();
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_COMMAND_MANAGER_H