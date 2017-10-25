// Undo Library
// Copyright (C) 2015-2017 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef UNDO_HISTORY_H_INCLUDED
#define UNDO_HISTORY_H_INCLUDED
#pragma once

namespace undo {

  class UndoCommand;
  class UndoState;

  class UndoHistoryDelegate {
  public:
    virtual ~UndoHistoryDelegate() { }
    virtual void onDeleteUndoState(UndoState* state) { }
  };

  class UndoHistory {
  public:
    UndoHistory(UndoHistoryDelegate* delegate = nullptr);
    virtual ~UndoHistory();

    const UndoState* firstState()   const { return m_first; }
    const UndoState* lastState()    const { return m_last; }
    const UndoState* currentState() const { return m_cur; }

    void add(UndoCommand* cmd);
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();

    // Deletes the whole redo history. Can be called before an add()
    // to create a linear undo history.
    void clearRedo();

    // Deletes the first UndoState. It can be useful to limit the size
    // of the undo history.
    bool deleteFirstState();

    // This can be used to jump to a specific UndoState in the whole
    // history.
    void moveTo(const UndoState* new_state);

  private:
    const UndoState* findCommonParent(const UndoState* a,
                                      const UndoState* b);
    void deleteState(UndoState* state);

    UndoHistoryDelegate* m_delegate;
    UndoState* m_first;
    UndoState* m_last;
    UndoState* m_cur;          // Current action that can be undone
  };

} // namespace undo

#endif  // HISTORY_H_INCLUDED
