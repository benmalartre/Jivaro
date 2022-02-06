#include "../command/manager.h"

PXR_NAMESPACE_OPEN_SCOPE

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
    command->Execute();
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
  command->Undo();                        // undo most recently executed command
}

void 
CommandManager::Redo() {
  if (_redoStack.size() <= 0) {
    return;
  }
  std::shared_ptr<Command> command = _redoStack.back();
  _undoStack.push_back(command);          // add undone command to redo stack
  _redoStack.pop_back();                  // remove top entry from redo stack
  command->Redo();                        // redo most recently executed command
}

void
CommandManager::Clear() {
  _undoStack.clear();
  _redoStack.clear();
}

PXR_NAMESPACE_CLOSE_SCOPE
