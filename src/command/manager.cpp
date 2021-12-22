#include "../command/manager.h"

PXR_NAMESPACE_OPEN_SCOPE

static void
_ExecuteCommandsThread(CommandManager* manager)
{
  while(true) manager->ExecuteCommands();
}

void 
CommandManager::Start()
{
  std::cout << "COMMAND MANAGER START WORKER THREAD..." << std::endl;
  _thread = std::thread(_ExecuteCommandsThread, this);
  std::cout << "COMMAND MANAGER THREAD STARTED..." << std::endl;
  std::cout << "COMMAND MANAGER JOIN CALLED..." << std::endl;
}

void
CommandManager::AddCommand(std::shared_ptr<Command> command)
{
  _mutex.lock();
  _todoStack.push_front(command);
  _mutex.unlock();
}

void
CommandManager::ExecuteCommands() 
{
    if(_todoStack.size()) {
        _mutex.lock();

        std::cout << "FOUND COMMAND !! DO IT :) !" << std::endl;
        _redoStack = CommandStack_t();             // clear the redo stack
        std::shared_ptr<Command> command = _todoStack.back();
        _todoStack.pop_back();
        _undoStack.push_back(command);
        _mutex.unlock();
        command->Execute();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void 
CommandManager::Undo() {
  if (_undoStack.size() <= 0) {
    std::cout << "NOTHING TO UNDO ..." << std::endl;
    return;
  }
  _mutex.lock();
  std::shared_ptr<Command> command = _undoStack.back();
  _redoStack.push_back(command);          // add undone command to undo stack
  _undoStack.pop_back();                  // remove top entry from undo stack
  _mutex.unlock();
  command->Undo();                        // undo most recently executed command
}

void 
CommandManager::Redo() {
  if (_redoStack.size() <= 0) {
    std::cout << "NOTHING TO REDO ..." << std::endl;
    return;
  }
  _mutex.lock();
  std::shared_ptr<Command> command = _redoStack.back();
  _undoStack.push_back(command);          // add undone command to redo stack
  _redoStack.pop_back();                  // remove top entry from redo stack
  _mutex.unlock();
  command->Redo();                        // redo most recently executed command
}

PXR_NAMESPACE_CLOSE_SCOPE
