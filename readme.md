# Nana IDE - an IDE for C++

This project aim, is to build an IDE for "modern C++", using only free tools.
The aim is to use free compilers, and libraries, using C++14 or 17.

The IDE will use as a build system CMake and will support conan. That means that
developers using this IDE will not have to deal with low level integration of
packages - just like modern languages do (like Go, Rust, Python and Java using
Gradle/Maven).

## History
I always tend to write "desktop" GUIs in Qt. However - it feels too bloated,
and not going to the same direction as standard C++ is. Qt was designed when
C++ did not even have STL - and much of its ideas are around it.

I started writing a GUI for an IDE (how original) kept going. How long can I go?

## Toolchain
The build system tested mainly is GCC, just because its available. I also test
using Clang on Linux. I also aim to use ccache by default, even on windows,
to speed up compilation. So - the IDE will:

```
if $BUILDIR not available
    mkdir $BUILDIR
    conan install
    cmake -S $SOURCE -B $BUILDIR

cmake -B $BUILDIR
```

Which means that the user of the IDE will need to edit `CMakeLists.txt` - and
`connanfile.txt` and the code will just flow. Just like in modern languages.

## Tasks

1. Layout/GUI
    1. I need to be able to put buttons on the right on the toolbar. I am no
       using a regular toolbar. (DONE, reach the basic limit of Nana)
       http://nanapro.org/en-us/forum/index.php?u=/topic/1349/gghelp-with-layout-design
    2. When adding an editor from the list box click handler, the tabs are
       displayed but no editor is available. It goes out of scope and dies?
    3. On startup - I zoom the window after its shown. How to do this before?
       How to save/restore geometry?
    4. We need a good editor.  I am unsure if Nana has something usable
    5. I am not against old fashio menus - but they are obsolete. The app should
       have an option to display the menus - but by default, on menus will be
       visible.
2. IDE's build system
    1. Nana needs to be built as a shared object/DLL. This fails on build.
       https://github.com/cnjinhao/nana/issues/601
    2. On Windows - I am unable to build png and jpeg libraries. Again - all
       these need to be shared.
    3. Need to investigate how to use clang on Windows.
    4. Identify how to use mingw-makefiles, this fails to work.
    6. Integrate https://github.com/TheLartians/Format.cmake into project default
3. Missing APIS
    1. Filesystem: add standard directories as Qt has.
    2. Sub shell running: need a class to start, stop (pause?) sub-proces, and
       read stdout/stderr - write stdin (needed?)
    3. How to detect the main target from a CMakeFile?
    4. On Linux - use native open file/directory dialogs
4. IDEs features
    1. Save open tabs
    2. New project wizard
    3. It would be nice to have markdown editor, we can use a small HTML library
       for rendering:  https://github.com/litehtml/litehtml
    4. Project wide search
    5. Git integration
    6. We need to integrate all the cool new clang tools: format and tidy.
    7. Intgrate a language server: https://clangd.llvm.org/installation.html, https://github.com/MaskRay/ccls
    8. Integrate a debugger, gdb/lldb
    9. Set run environment: PATH, LD_LIBRARY_PATH, and command line arguments for
       application being developed
5. Usage/installation documentation
    1. It would be nice if on runtime that would detect what needs to be
       installed, and would install it on demand (you are missing CMake,
       CCache and a compiler -> click here to install them).
