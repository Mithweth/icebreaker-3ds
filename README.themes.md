# Themes

IceBreaker themes are simple to create. They're are definined in text files
with the extension .ibt (that's _I_ce_B_reaker _T_heme) in the IceBreaker
data directory. The filename is the name of the theme itself, which must be
seven characters or less -- not because of the DOS 8.3 limit but because
more than seven requires the options menubox to be really big and ugly and
we don't want that. 

The file consists mostly of color definitions, and also gives the location
of the sound files and the one sprite bitmap. Color definitions begin
with a keyword ("Background", "NormalText", etc.) and then give decimal
values from 0-255 for red, green, and blue, respectively. 

The sound and image defintions are just the keyword followed by a filename.
More info on file formats in a bit.

The keywords aren't case sensitive, and you can use any amount of whitespace
you want. If you leave out a particular keyword, IceBreaker will use the
default but spit out a complaint on stderr -- so don't do that. (There's
currently one exception -- SpriteMirrorBitmap, which I'll get to in a bit.)

All four sounds (SoundOuch, SoundCrash, SoundVictory and SoundDefeat) are
optional: a theme without them is just silent for those events, with no
complaint.


The default theme is "linux", and the file linux.ibt looks approximately
like this:
```
  Background              0   64  128
  NormalText            255  255  255
  BoardFillMin          224  224  224
  BoardFillMax          255  255  255
  Gridline              192  192  192
  GridHighlight         192  192  192
  Line1                   0    0    0
  Line2                 192    0   64
  MenuHighlight         255  255  255
  GameoverText          192    0   64
  ScorescrollText         0    0    0
  BonusscrollText       192    0   64
  TextentryBox            0    0    0
  TextentryText         255  255  255
  CopyrightText         192  192  192
  ProgressbarOutline  180 225 245
  ProgressbarInner    20  50  75
  ProgressbarFill     100  200  240
  SpriteBitmap          penguin
  #FallingParticle      particle
  SoundOuch             ouch
  SoundCrash            crash
  SoundVictory          victory
  SoundDefeat           defeat
```
The names should mostly be self-explanatory. If something is unclear, let me
know, and I'll either change the name in a future version or write better
documentation here. A few things bear mentioning:

BoardFillMin and BoardFillMax define the colors of the game play area. The
values will be randomly selected from the range given, but there's a twist
-- if two (or three) of the red, green, or blue values have identical Min
*and* identical Max values, those two will be "locked" together. This way,
you can produce grey or monotone backgrounds instead of having every theme
look like the mutant love child of Rainbow Brite and the Swamp Thing.

The bitmap file should be a 24-bit .png format. Alpha is supported.
The SpriteBitmap should be 14x14. If you want, you can make them as
small as 12x12, but that's basically treated as the top-left of 14x14,
and support for that may vanish in a future version.
(If you want smaller, just mask out most of the image.)
BTW, do not give the .png extension in the ibt file.
The FallingParticle must be tiny: from 5x5 to 9x9.

Don't underestimate how hard it is to make a sprite image look good at such
a tiny size. Making a theme which represents something in real life rather
than being abstract or iconic is a challenge. A future version of IceBreaker
may allow you to include background and playing board graphics instead of
just colors -- but don't hold your breath for that.

The sound files are 16 bit mono PCM at 22050. They should also be really
short -- under a second. This'll mean the total size will be easily less
than 30k -- if it's not, something's not right.
They are expected to be converted as RAW files :
```
ffmpeg -i "<wav file>" -ac 1 -ar 22050 -f s16le "assets/sounds/<raw file>"
```
Do not give the .png extension in the ibt file.

If you do produce a good theme, let me know -- send it to 
<mattdm@mattdm.org>, and I'll consider including it with the main
distribution. All themes must be licensed under the GPL.
