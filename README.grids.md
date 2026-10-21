# Grids

A grid sets the shape of the iceberg: which parts of the play area are ice
and which are open sea from the start of each level. Grids are plain text
files with the extension `.ibg` (_I_ce_B_reaker _G_rid) in `assets/grids/`.


To create your own iceberg, just fill your files with `#`s (the ice), and `.`s
(the open sea), and you are ready to roll !

Each character covers a square of 2 × 2 blocks (20 × 20 pixels) on the
bottom screen, so the 16 × 12 characters fill the whole 320 × 240 screen.

## Limitations

- The name must be a **single word** of 31 characters or less: it is saved
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
