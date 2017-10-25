// Undo Library
// Copyright (C) 2015-2017 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "test.h"

#include "cmd.h"
#include "undo_history.h"

using namespace undo;

int main(int argc, char** argv)
{
  // 1 --- 2 --- 3 --- 4   -------- 7 --- 8
  //       |              |
  //        ------------- 5 --- 6
  int model = 0;
  Cmd cmd1(model, 1, 0);
  Cmd cmd2(model, 2, 1);
  Cmd cmd3(model, 3, 2);
  Cmd cmd4(model, 4, 3);
  Cmd cmd5(model, 5, 2);
  Cmd cmd6(model, 6, 5);
  Cmd cmd7(model, 7, 5);
  Cmd cmd8(model, 8, 7);

  UndoHistory history;
  cmd1.redo(); history.add(&cmd1);
  cmd2.redo(); history.add(&cmd2);
  cmd3.redo(); history.add(&cmd3);
  cmd4.redo(); history.add(&cmd4);
  history.undo();
  history.undo();
  cmd5.redo(); history.add(&cmd5);
  cmd6.redo(); history.add(&cmd6);
  history.undo();
  cmd7.redo(); history.add(&cmd7);
  cmd8.redo(); history.add(&cmd8);

  EXPECT_EQ(8, model);
  history.undo(); EXPECT_EQ(7, model);
  history.undo(); EXPECT_EQ(6, model);
  history.undo(); EXPECT_EQ(5, model);
  history.undo(); EXPECT_EQ(4, model);
  history.undo(); EXPECT_EQ(3, model);
  history.undo(); EXPECT_EQ(2, model);
  history.undo(); EXPECT_EQ(1, model);
  history.undo(); EXPECT_EQ(0, model);
  EXPECT_FALSE(history.canUndo());

  history.redo(); EXPECT_EQ(1, model);
  history.redo(); EXPECT_EQ(2, model);
  history.redo(); EXPECT_EQ(3, model);
  history.redo(); EXPECT_EQ(4,  model);
  history.redo(); EXPECT_EQ(5,  model);
  history.redo(); EXPECT_EQ(6,  model);
  history.redo(); EXPECT_EQ(7,  model);
  history.redo(); EXPECT_EQ(8,  model);
  EXPECT_FALSE(history.canRedo());

  // Delete states ----------------------------------------

  auto undoAll = [&]{
    while (history.canUndo())
      history.undo();
  };
  auto redoAll = [&]{
    while (history.canRedo())
      history.redo();
  };

  // Cannot delete state 1 if we are in that state
  // 1 --- 2 --- 3 --- 4   -------- 7 --- 8
  //       |              |
  //        ------------- 5 --- 6
  undoAll(); EXPECT_EQ(0, model);
  history.redo(); EXPECT_EQ(1, model);
  EXPECT_FALSE(history.deleteFirstState());
  redoAll(); EXPECT_EQ(8, model);

  // Delete state 1
  // x --- 2 --- 3 --- 4   -------- 7 --- 8
  //       |              |
  //        ------------- 5 --- 6
  EXPECT_TRUE(history.deleteFirstState());

  EXPECT_EQ(8, model);
  history.undo(); EXPECT_EQ(7, model);
  history.undo(); EXPECT_EQ(6, model);
  history.undo(); EXPECT_EQ(5, model);
  history.undo(); EXPECT_EQ(4, model);
  history.undo(); EXPECT_EQ(3, model);
  history.undo(); EXPECT_EQ(2, model);
  history.undo(); EXPECT_EQ(1, model);
  EXPECT_FALSE(history.canUndo());
  redoAll(); EXPECT_EQ(8, model);

  // Delete state 2 (we cannot go back to 3 or 4 from 5 because 2 is gone)
  // x --- x --- x --- x   -------- 7 --- 8
  //       |              |
  //        ------------- 5 --- 6
  EXPECT_TRUE(history.deleteFirstState());

  EXPECT_EQ(8, model);
  history.undo(); EXPECT_EQ(7, model);
  history.undo(); EXPECT_EQ(6, model);
  history.undo(); EXPECT_EQ(5, model);
  history.undo(); EXPECT_EQ(2, model);
  EXPECT_FALSE(history.canUndo());
  redoAll(); EXPECT_EQ(8, model);

  // Delete state 5 (we cannot go back to 6 from 7 because 5 is gone)
  // x --- x --- x --- x   -------- 7 --- 8
  //       |              |
  //        ------------- x --- x
  EXPECT_TRUE(history.deleteFirstState());

  EXPECT_EQ(8, model);
  history.undo(); EXPECT_EQ(7, model);
  history.undo(); EXPECT_EQ(5, model);
  EXPECT_FALSE(history.canUndo());
  redoAll(); EXPECT_EQ(8, model);

  // Delete state 7
  // x --- x --- x --- x   -------- x --- 8
  //       |              |
  //        ------------- x --- x
  EXPECT_TRUE(history.deleteFirstState());

  EXPECT_EQ(8, model);
  history.undo(); EXPECT_EQ(7, model);
  EXPECT_FALSE(history.canUndo());
  history.redo(); EXPECT_EQ(8, model);

  // Cannot delete state 8 because is the current state
  // x --- x --- x --- x   -------- x --- 8
  //       |              |
  //        ------------- x --- x
  EXPECT_FALSE(history.deleteFirstState());
  EXPECT_EQ(8, model);
  history.undo(); EXPECT_EQ(7, model);
  // Now we can delete the whole history
  EXPECT_TRUE(history.deleteFirstState());
  EXPECT_FALSE(history.canUndo());
  EXPECT_FALSE(history.canRedo());
}
