// Undo Library
// Copyright (C) 2015-2022 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "undo_history.h"

#include "undo_command.h"
#include "undo_state.h"

#include <cassert>
#include <stack>

#define UNDO_TRACE(...)

namespace undo {

UndoHistory::UndoHistory(UndoHistoryDelegate* delegate)
  : m_delegate(delegate)
  , m_first(nullptr)
  , m_last(nullptr)
  , m_cur(nullptr)
{
}

UndoHistory::~UndoHistory()
{
  m_cur = nullptr;
  clearRedo();
}

bool UndoHistory::canUndo() const
{
  return m_cur != nullptr;
}

bool UndoHistory::canRedo() const
{
  return m_cur != m_last;
}

void UndoHistory::undo()
{
  assert(m_cur);
  if (!m_cur)
    return;

  assert(
    (m_cur != m_first && m_cur->m_prev) ||
    (m_cur == m_first && !m_cur->m_prev));

  moveTo(m_cur->m_prev);
}

void UndoHistory::redo()
{
  if (!m_cur)
    moveTo(m_first);
  else
    moveTo(m_cur->m_next);
}

void UndoHistory::clearRedo()
{
  UndoState* state = m_last;

  // First we break the chain of undo states because deleteState() can
  // generate an onDeleteUndoState() notification which might try to
  // iterate over the whole UndoHistory.
  if (m_cur) {
    m_cur->m_next = nullptr;
    m_last = m_cur;
  }
  else {
    m_first = m_last = nullptr;
  }

  for (UndoState* prev = nullptr;
       state && state != m_cur;
       state = prev) {
    prev = state->m_prev;
    deleteState(state);
  }
}

bool UndoHistory::deleteFirstState()
{
  UNDO_TRACE("UndoHistory::deleteFirstState()\n");

  // We cannot delete the first state if we are in the first state.
  if (m_cur == m_first) {
    UNDO_TRACE(" - Cannot delete first state if it's the current state\n");
    return false;
  }

  UndoState* i = m_last;
  while (i) {
    // If this state depends on the delete one, this "i" is the new
    // m_first undo state.
    if (i->m_parent == m_first) {
      // First we check if the current undo state is one of the states
      // that we're going to delete.
      UndoState* j = m_first;
      while (j != i) {
        // Cannot delete this "j" state because is the current one.
        if (m_cur == j) {
          UNDO_TRACE(" - Cannot delete first state because current state depends on it to go to the last state\n");
          return false;
        }
        j = j->next();
      }

      j = m_first;
      while (j != i) {
        UNDO_TRACE(" - Delete undo state\n");

        UndoState* k = j;
        j = j->next();

        deleteState(k);
      }

      i->m_prev = nullptr;
      i->m_parent = nullptr;
      m_first = i;
      return true;
    }
    i = i->prev();
  }

  UndoState* state = m_first;
  assert(m_last == m_first);
  assert(m_first->next() == nullptr);
  m_first = m_last = nullptr;
  UNDO_TRACE(" - Delete first state only\n");

  deleteState(state);
  return true;
}

void UndoHistory::add(UndoCommand* cmd)
{
  UndoState* state = new UndoState(cmd);
  state->m_prev = m_last;
  state->m_next = nullptr;
  state->m_parent = m_cur;

  if (!m_first)
    m_first = state;

  m_cur = m_last = state;

  if (state->m_prev) {
    assert(!state->m_prev->m_next);
    state->m_prev->m_next = state;
  }
}

const UndoState* UndoHistory::findCommonParent(const UndoState* a,
                                               const UndoState* b)
{
  const UndoState* pA = a;
  const UndoState* pB = b;

  if (pA == nullptr || pB == nullptr)
    return nullptr;

  while (pA != pB) {
    pA = pA->m_parent;
    if (!pA) {
      pA = a;
      pB = pB->m_parent;
      if (!pB)
        return nullptr;
    }
  }

  return pA;
}

void UndoHistory::moveTo(const UndoState* new_state)
{
  const UndoState* common = findCommonParent(m_cur, new_state);

  if (m_cur) {
    while (m_cur != common) {
      m_cur->m_cmd->undo();
      m_cur = m_cur->m_parent;
    }
  }

  if (new_state) {
    std::stack<const UndoState*> redo_parents;
    const UndoState* p = new_state;
    while (p != common) {
      redo_parents.push(p);
      p = p->m_parent;
    }

    while (!redo_parents.empty()) {
      p = redo_parents.top();
      redo_parents.pop();

      p->m_cmd->redo();
    }
  }

  m_cur = const_cast<UndoState*>(new_state);
}

void UndoHistory::deleteState(UndoState* state)
{
  if (m_delegate)
    m_delegate->onDeleteUndoState(state);

  delete state;
}

} // namespace undo
