Challenge Me: Word Puzzles
==========================

A decompilation of the Slam Technology Wii Runtime from *Challenge Me: World Puzzles*.

This repository does not contain any of the original objects. An existing copy of the game [`[SC5PGN]`](https://wiki.dolphin-emu.org/index.php?title=SC5PGN) *Challenge Me: Word Puzzles* is required. (See [Building](#building) below.)

Supported versions:
- `20100817_DBG` - 2010 August 17th debug build
  - Revolution SDK debug build: 2009 December 11th
  - Revolution Face Library debug build: 2008 March 6th
  - HOME Button Library debug build: 2010 February 24th
- `20100817_REL` - 2010 August 17th release build
  - Revolution SDK release build: 2009 December 11th
  - Revolution Face Library release build: 2008 March 6th
  - HOME Button Library release build: 2010 February 24th
- `20090820_REL` - 2009 August 20th release build
  - Revolution SDK release build: 2009 February 24th
  - Revolution Face Library release build: 2008 March 6th
  - HOME Button Library release build: 2008 September 24th

Dependencies
============

Windows
--------

On Windows, it's **highly recommended** to use native tooling. WSL or msys2 are **not** required.  
When running under WSL, [objdiff](#diffing) is unable to get filesystem notifications for automatic rebuilds.

- Install [Python](https://www.python.org/downloads/) and add it to `%PATH%`.
  - Also available from the [Windows Store](https://apps.microsoft.com/store/detail/python-311/9NRWMJP3717K).
- Download [ninja](https://github.com/ninja-build/ninja/releases) and add it to `%PATH%`.
  - Quick install via pip: `pip install ninja`

macOS
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages):

  ```sh
  brew install ninja
  ```

[wibo](https://github.com/decompals/wibo), a minimal 32-bit Windows binary wrapper, will be automatically downloaded and used.

Linux
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).

[wibo](https://github.com/decompals/wibo), a minimal 32-bit Windows binary wrapper, will be automatically downloaded and used.

Building
========

- Clone the repository:

  ```sh
  git clone https://github.com/yomcube/cmwp-decomp.git
  ```

- Copy your disc image of *Challenge Me: Word Puzzles* to `orig`.
  - Supported formats: ISO (GCM), RVZ, WIA, WBFS, CISO, NFS, GCZ, TGC
  - After the initial build, the disc image can be deleted to save space.

- Configure:

  ```sh
  python configure.py
  ```

  `20100817_DBG` is the default version. To use a different version, specify it with `--version`.

- Build:

  ```sh
  ninja
  ```

Diffing
=======

Once the initial build succeeds, an `objdiff.json` should exist in the project root.

Download the latest release from [encounter/objdiff](https://github.com/encounter/objdiff). Under project settings, set `Project directory`. The configuration should be loaded automatically.

Select an object from the left sidebar to begin diffing. Changes to the project will rebuild automatically: changes to source files, headers, `configure.py`, `splits.txt` or `symbols.txt`.

![](assets/objdiff.png)

Thanks to
=========
[doldecomp/dolsdk2004](github.com/doldecomp/dolsdk2004) - Decompilation of Dolphin SDK 2004 (which is what RVL_SDK is based on)
