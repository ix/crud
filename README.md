# crud
crud (C Rectangular oUtline Drawer) is a tool which lets you select a region on the screen and prints information about that region to stdout.

It allows changing the selection border size and the color of the selection at compile time through its configuration file, `config.h`. It tries hard not to do anything unnecessary, consequently, the source code is very small and readable. Its output is very much like [slop](https://github.com/naelstrof/slop), however `crud` is much smaller.
