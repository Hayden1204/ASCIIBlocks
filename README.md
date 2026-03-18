# ASCIIBlocks

**ASCIIBlocks** is an open-world, block-based game that runs in your terminal! You can create builds within finite maps, which you can save and share with other people.

A GUI client is planned, codenamed _Acaciablocks_.

## How to play

Use the WASD keys to move, and use IJKL to place blocks. To change your held block, use the keys F and H. To open the menu, use the M key.

## Compiling (Windows)

_TODO_

## Compiling (Linux, *BSD, MacOS)

### The modern way:
```sh
meson setup build .
meson compile -C build
```

### The old way:
To compile an executable named `asciiblocks` using `gcc`, optimised for your hardware:

`gcc asciiblocks.c -O2 -flto -s -march=native -o asciiblocks -lncurses`

## Cross-compiling to windows (from Linux, *BSD, MacOS):
```sh
meson setup --cross-file windows-crosscompilation.txt build-mingw
meson compile -C build-mingw
```
