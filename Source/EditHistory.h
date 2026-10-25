#pragma once
#include <vector>
#include <cstddef>

// No fixed entry cap. Memory use grows with the number of edits.
template <typename Snapshot> class EditHistory
{
public:
    void record(const Snapshot& value)
    {
        if (!entries.empty() && entries[position] == value) return;
        if (!entries.empty()) entries.resize(position + 1);
        entries.push_back(value);
        position = entries.size() - 1;
    }
    bool canUndo() const { return !entries.empty() && position > 0; }
    bool canRedo() const { return !entries.empty() && position + 1 < entries.size(); }
    const Snapshot* undo() { return canUndo() ? &entries[--position] : nullptr; }
    const Snapshot* redo() { return canRedo() ? &entries[++position] : nullptr; }
    void clear() { entries.clear(); position = 0; }
private:
    std::vector<Snapshot> entries;
    size_t position = 0;
};
