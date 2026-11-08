# arex - file system at your fingertips - literally. 

[AR]ow [EX]lorer: Fast ergonomic inline terminal navigation utility driven with arrow or awsd keys.

Written in C++20 with high performance techniques to make the tool very crisp and satisfying.

No external 3rd party lib dependencies. Tested on Debian.

## TODO:

Test on Mac and Windows

## Quick Preview

Navigate the dir tree in the terminal with arrow keys, awsd or vim keys

```
 [arex] /home/mishar/buildingcpp/concurrency/src
┌─ Preview ──────────────────────────────────────────
│ add_subdirectory(library)
│ add_subdirectory(executable)
│ add_subdirectory(test)
│
│
│
│
│
└────────────────────────────────────────────────────
   [DIR]    executable/
   [DIR]    library/
   [DIR]    test/
 > [FILE]   CMakeLists.txt
   [ACTION] [Actions...]
```

Run core basic commands instantly.

```
 [arex] /home/mishar/buildingcpp/concurrency/src
┌─ Preview ──────────────────────────────────────────
│ add_subdirectory(library)
│ add_subdirectory(executable)
│ add_subdirectory(test)
│
│
│
│
│
└────────────────────────────────────────────────────
 > [ACTION] Copy Relative Path
   [ACTION] Copy Full Path
   [ACTION] default (xdg-open)
   [ACTION] cat
   [ACTION] more
   [ACTION] less
   [ACTION] cut
   [ACTION] copy
 -- [File Actions] Target: CMakeLists.txt (Esc to cancel)
```

## Subtle nod to my favorite car

The mighty Mazda RX3

https://en.wikipedia.org/wiki/Mazda_Grand_Familia#/media/File:Mazda-Savanna.JPG
