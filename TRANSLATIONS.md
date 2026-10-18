# Translations

IceBreaker 3DS supports languages other than English. Adding
a language only takes one text file: no code change is needed.

## Adding a language

1. Copy `assets/lang/en.lang` to a new file in the same folder, for example
   `assets/lang/es.lang`.
2. Change the two header lines:
   ```
   NAME=espanol
   ORDER=50
   ```
   - `NAME` is shown in the **Language** menu and saved in the options file.
     It must be a **single word** of 31 bytes or less, with no spaces.
   - `ORDER` sets the position in the menu, from lowest to highest.
3. Translate every value, after the `=` sign. Leave the keys, before the
   `=`, unchanged.
4. Rebuild with `make`. The file is copied to the RomFS automatically, and
   the new language should appear in **Menu → Language**.

## File format

- One `KEY=value` pair per line, UTF-8 encoded.
- Empty lines and lines starting with `#` are ignored.
- A line can't be longer than 127 bytes, key included.
- If a key is missing, the game shows the key itself (for example
  `MENU_NEW_GAME`) instead of the text. Keep all the keys from `en.lang`.

## Characters

The game uses its own pixel font, so many characters are not supported.
In a nutshell, English, French, Deutsch and Italian specific characters exist
but you will need to update `text.c` to add a language with specific glyphs.

## Text length

The screens are small, so keep translations about as short as the English
text.
