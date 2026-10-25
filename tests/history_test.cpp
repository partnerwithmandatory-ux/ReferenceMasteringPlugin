#include "../Source/EditHistory.h"
#include <cassert>
int main()
{
    EditHistory<int> history;
    assert(!history.canUndo() && !history.canRedo());
    history.record(0);
    for(int i=1;i<=100000;++i) history.record(i);
    history.record(100000); // Duplicate edits do not add an undo step.
    for(int i=99999;i>=0;--i) { auto* value=history.undo(); assert(value && *value==i); }
    assert(!history.canUndo());
    for(int i=1;i<=100000;++i) { auto* value=history.redo(); assert(value && *value==i); }
    assert(!history.canRedo());
    history.undo(); history.record(-1); assert(!history.canRedo());
    assert(*history.undo()==99999); assert(*history.redo()==-1);
    history.clear(); assert(!history.canUndo() && !history.canRedo());
}
