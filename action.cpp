#include "action.h"

Action::Action(functionPtr undo, functionPtr redo)
{
    this->undo=undo;
    this->redo=redo;
}

Action::executeUndo(){
    this->undo();
}

Action::executeRedo(){
    this->redo();
}
