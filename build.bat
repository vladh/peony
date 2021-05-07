@echo off

:: Compiler flags
:: ---------------------
:: -FC - Display full path of source code files in warnings
:: -GL - Enable whole program optimisation
:: -GS - Add debugging info to object files
:: -MTd - Use the multithreaded, static version of the runtime library and define _DEBUG
:: -MP4 - Use 4 CPU cores
:: -O2 - Maximise speed
:: -Oi - Generate intrinsic functions to run faster
:: -W4 - Show most warnings
:: -WX - Turn warnings into errors
:: -WL - Append second line of info to errors
:: -Z7 - Add debugging info to object files
:: -Zo - Generate debugging info for optimised code
:: -diagnostics:column - Display more error/warning info
:: -fp:fast - Simplify floating point precision to run faster
:: -nologo - Remove banner message
:: -sdl - Enable additional security checks
:: -guard:cf - Add control flow guard security checks
:: -analyze - Enable static analysis
:: -permissive- - Specify standards conformance mode to the compiler
:: -fsanitize=address - Enable AddressSanitizer

:: Linker flags
:: ---------------------
:: -STACK:reserve[,commit] - Stack size
:: -INCREMENTAL - Link incrementally
:: -DEBUG - Put debug information in the .pdb file
:: -opt:ref - Remove functions and data that are never referenced

echo ################################################################################
echo ### Building
echo ################################################################################

if not defined DevEnvDir (
  call vcvarsall x64
)

set compiler_flags=/I "C:/opt/include/" ^
-FC ^
-GS ^
-MTd ^
-MP18 ^
-W4 ^
-WL ^
-WX ^
-Z7 ^
-Zo ^
-diagnostics:column ^
-fp:fast ^
-nologo ^
-sdl ^
-guard:cf ^
-analyze ^
-permissive- ^
-fsanitize=address ^
-std:c++latest ^
-wd4100 -wd4127 -wd4201 ^
/D_ITERATOR_DEBUG_LEVEL=0 ^
-O2
:: -GL
:: -Oi

set linker_flags=/LIBPATH:"C:/opt/lib/" ^
-INCREMENTAL:NO ^
-CGTHREADS:8 ^
-DEBUG:FULL ^
-opt:ref ^
/NODEFAULTLIB:msvcrt.lib ^
freetype.lib opengl32.lib glfw3.lib assimp-vc142-mtd.lib user32.lib gdi32.lib shell32.lib
:: -STACK:0x100000,0x100000

pushd build

cl %compiler_flags% ..\src\peony.cc ^
/link /SUBSYSTEM:console %linker_flags% /out:peony.exe

popd
