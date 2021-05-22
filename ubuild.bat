@echo off

echo ################################################################################
echo ### Building
echo ################################################################################

ctime -begin peony.ctm

if not defined DevEnvDir (
  call vcvarsall x64
)

set compiler_flags=/I "C:/local/include/" ^
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
-permissive- ^
-std:c++latest ^
-wd4100 -wd4127 -wd4201 ^
-O2 ^
-GL ^
-Oi

set linker_flags=/LIBPATH:"C:/local/lib/" ^
-CGTHREADS:8 ^
-DEBUG:FULL ^
-INCREMENTAL:NO ^
-opt:ref ^
-NODEFAULTLIB:msvcrt.lib ^
freetype.lib opengl32.lib glfw3.lib assimp-vc142-mtd.lib user32.lib gdi32.lib shell32.lib

pushd bin

cl %compiler_flags% ..\src\_unity.cpp ^
/link /SUBSYSTEM:console %linker_flags% /out:peony.exe

popd

ctime -end peony.ctm %LastError%
