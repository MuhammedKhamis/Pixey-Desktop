#ifndef ACTION_H
#define ACTION_H

typedef void (*functionPtr)();

class Action
{
private:
    functionPtr undo;
    functionPtr redo;
public:
    Action(functionPtr undo,functionPtr redo);
    void executeUndo();
    void executeRedo();

};

#endif // ACTION_H
