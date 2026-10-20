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

## High scores

High scores are kept separately for each grid, under the grid's name. If you
rename a grid file, its high scores are lost.
