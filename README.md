# IceBreaker 3DS

A Nintendo 3DS port of [IceBreaker](http://www.mattdm.org/icebreaker/), the
JezzBall-like puzzle game by Matthew Miller.

Penguins bounce around an iceberg. Draw lines across the ice to cut it into
pieces: every piece left without a penguin falls into the sea. Clear 80% of
the iceberg to move on to the next level, which has one more penguin and gives
you one more life. If a penguin touches a line while it is still being drawn,
the line breaks and you lose a life.

The more ice you clear beyond 80%, the bigger the bonus, and you also get a
small bonus for the lives you have left. But don't take too long: your score
for the level drops by one point every second.

For the full story (and the original game's humour), see the
[original README](README.original).

## Controls

The game is played on the bottom screen. The top screen shows the level,
score, high score, lives and how much ice has been cleared.

| Input                  | Action                                                                   |
|------------------------|--------------------------------------------------------------------------|
| Swipe with the stylus  | Start two lines from where the swipe begins: a vertical swipe draws up and down, a horizontal swipe draws left and right |
| Swipe on a penguin     | "Ouch!" — nothing happens, no life lost                                  |
| Tap the touch screen   | Skip the intro, or continue after the end-of-level screen                |
| START                  | Open / close the menu (pauses the game)                                  |
| D-Pad up / down        | Move through the menu                                                    |
| D-Pad left / right     | Change the difficulty or the theme                                       |
| A                      | Confirm                                                                  |

## Menu

- **New game** — start over from level 1.
- **Difficulty** — easy, normal or hard. Changing it ends the current game
  (you are asked to confirm).
- **Theme** — change the look of the game. Seven themes are included:
  blocks, cows, jz-like, kitty, linux (default), stars and turtles.
- **High scores** — the five best scores. When you set a new one, you are
  asked for your name with the system keyboard.
- **Help** — a short summary of the rules.

Options and high scores are saved on the SD card, in `sdmc:/.icebreaker` and
`sdmc:/.icebreaker.scores`.

## Installation

Download the latest `.3dsx` or `.cia` from the
[releases page](https://github.com/Mithweth/icebreaker-3ds/releases).

- **`.3dsx`** — copy it to the `/3ds/` folder of your SD card and start it
  from the Homebrew Launcher.
- **`.cia`** — install it with a CIA installer such as FBI. The game then
  appears on the HOME Menu.

## Building

You need [devkitPro](https://devkitpro.org/) with devkitARM, libctru,
citro2d and tex3ds. There are two ways to get them.

### With Docker

`build_docker.sh` runs the build inside the official
`devkitpro/devkitarm` image, so nothing has to be installed locally:

```sh
./build_docker.sh build        # build icebreaker-3ds.3dsx
./build_docker.sh              # clean, build, then install over the network
./build_docker.sh -d build     # debug build (see below)
```

To send the game to the console with `3dslink`, start the Homebrew Launcher,
press Y to wait for a network connection, then:

```sh
NITRO_IP=192.168.1.42 ./build_docker.sh install
```

### With a local toolchain

On Debian or Ubuntu, `install-3ds-env.sh` installs devkitPro, the 3DS
libraries, `bannertool` and `makerom` (it needs root):

```sh
sudo ./install-3ds-env.sh
source /etc/profile.d/devkit-env.sh
```

Then:

| Command       | Result                                                                  |
|---------------|-------------------------------------------------------------------------|
| `make`        | Build `icebreaker-3ds.3dsx`                                              |
| `make banner` | Build the HOME Menu banner (needed once before `make cia`)              |
| `make cia`    | Build `icebreaker-3ds.cia` (run `make` first; version set with `APP_VERSION=x.y.z`) |
| `make run`    | Start the game in the [Azahar](https://azahar-emu.org/) emulator (Flatpak) |
| `make lint`   | Build with GCC's static analyser (`-fanalyzer`)                          |
| `make clean`  | Remove all build outputs                                                |

### Debug builds

Building with `DEBUG=1` (or `./build_docker.sh -d`) redirects `stdout` and
`stderr` to the computer that sent the game: run `3dslink -s` to see the
game's log messages.

## Themes

Themes are plain text files in `assets/themes/`. They set the colours, the
penguin sprite and the sounds. See [README.themes.md](README.themes.md) to
write your own.

## Credits

- **Original game:** Matthew Miller, with many contributions from
  Enrico Tassi, and others listed in the [original README](README.original).
- **Nintendo 3DS port:** Jean-Baptiste Langlois.

## License

IceBreaker is free software, released under the
[GNU General Public License, version 2 or later](LICENSE).
