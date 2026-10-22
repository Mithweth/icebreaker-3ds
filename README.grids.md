# Grids

A grid sets the shape of the iceberg: which parts of the play area are ice
and which are open sea from the start of each level. Grids are plain text
files with the extension `.ibg` (_I_ce_B_reaker _G_rid) in `assets/grids/`.


To create your own iceberg, just fill your files with `#`s (the ice), and `.`s
(the open sea), and you are ready to roll !

Each character covers a square of 2 × 2 blocks (20 × 20 pixels) on the
bottom screen, so the 16 × 12 characters fill the whole 320 × 240 screen.

## Grid editor

You can also draw grids on the console with the **Editor** entry of the
main menu. Each square of the editor stands for one character of the grid.

- **Draw**: tap a square to toggle it between ice and sea, or drag the
  stylus to paint several squares the same way as the first one.
- **Clear**: empty the whole grid.
- **Save**: tap the name field to change the name, then confirm.
  12 characters at most. An empty grid can't be saved.
- **Load**: pick a saved grid with Up/Down and load it with A. Press Select
  to delete it.
- **Back** (or Start): leave the editor.

Saved grids go to `sdmc:/icebreaker/` as `.ibg` files. They show up in the
Grid option next to the built-in ones, and the Random grid can pick them too.

- You can keep up to 16 saved grids.
- The names of built-in grids, and `random`, can't be used.
- Saving over or deleting the grid you are playing restarts the game. A
  deleted grid is replaced by `classic`.

## Limitations

- The name must be a **single word** of 12 characters or less: it is saved
  in the options file and in the high score file with no extension.
- Grids whose name is greater than 8 characters will probably not fit in
  the options menu.
- `random` is reserved for the Random option of the grid menu: a grid file
  named `random.ibg` can't be played directly, and its high scores are never
  saved.

## High scores

High scores are kept separately for each grid, under the grid's name. If you
rename a grid file, its high scores are lost.

Games played with the Random grid don't record high scores: the board
changes every level, so the scores wouldn't belong to any single grid.
