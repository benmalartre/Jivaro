#include "../command/manager.h"

JVR_NAMESPACE_OPEN_SCOPE

// singleton
//----------------------------------------------------------------------------
CommandManager* CommandManager::_singleton=nullptr;

CommandManager* CommandManager::Get() { 
  if(_singleton==nullptr){
        _singleton = new CommandManager();
    }
    return _singleton; 
};

void
CommandManager::AddCommand(std::shared_ptr<Command> command)
{
  _todoStack.push_front(command);
}

void
CommandManager::ExecuteCommands() 
{
  if(_todoStack.size()) {
    _redoStack = CommandStack_t();             // clear the redo stack
    std::shared_ptr<Command> command = _todoStack.back();
    _todoStack.pop_back();
    _undoStack.push_back(command);
  }
}

void 
CommandManager::Undo() {
  if (_undoStack.size() <= 0) {
    return;
  }
  std::shared_ptr<Command> command = _undoStack.back();
  _redoStack.push_back(command);          // add undone command to undo stack
  _undoStack.pop_back();                  // remove top entry from undo stack
  command->Do();                        // undo most recently executed command
}

void 
CommandManager::Redo() {
  if (_redoStack.size() <= 0) {
    return;
  }
  std::shared_ptr<Command> command = _redoStack.back();
  _undoStack.push_back(command);          // add undone command to redo stack
  _redoStack.pop_back();                  // remove top entry from redo stack
  command->Do();                        // redo most recently executed command
}

void
CommandManager::Clear() {
  _undoStack.clear();
  _redoStack.clear();
}

JVR_NAMESPACE_CLOSE_SCOPE
