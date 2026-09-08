// Undo Library
// Copyright (C) 2026 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "test.h"

#include "cmd.h"
#include "undo_history.h"

using namespace undo;

int main(int argc, char** argv)
{
  // 1 --- 2 --- [we're here] --- 3 --- 4
  int model = 2;
  Cmd cmd1(model, 1, 0);
  Cmd cmd2(model, 2, 1);
  Cmd cmd3(model, 3, 2);
  Cmd cmd4(model, 4, 3);

  UndoHistory history;
  history.add(&cmd1);
  history.add(&cmd2);
  history.add(&cmd3, history.lastState(), false); // Don't move current state
  history.add(&cmd4, history.lastState(), false);

  EXPECT_EQ(2, model);
  history.redo(); EXPECT_EQ(3, model);
  history.redo(); EXPECT_EQ(4, model);
  history.undo(); EXPECT_EQ(3, model);
  history.undo(); EXPECT_EQ(2, model);
  history.undo(); EXPECT_EQ(1, model);
}
