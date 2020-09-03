@echo off

:: Compiler flags
:: ---------------------
:: -FC - Display full path of source code files in warnings
:: -GL - Enable whole program optimisation
:: -GS - Add debugging info to object files
:: -MTd - Use the multithreaded, static version of the runtime library and define _DEBUG
:: -O2 - Maximise speed
:: -Oi - Generate intrinsic functions to run faster
:: -W4 - Show most warnings
:: -WL - Append second line of info to errors
:: -Z7 - Add debugging info to object files
:: -Zo - Generate debugging info for optimised code
:: -diagnostics:column - Display more error/warning info
:: -fp:fast - Simplify floating point precision to run faster
:: -nologo - Remove banner message
:: -sdl - Enable additional security checks

set CommonCompilerFlags=/I "C:/opt/include/" ^
-FC ^
-GL ^
-GS ^
-MTd ^
-O2 ^
-Oi ^
-W4 ^
-WL ^
-Z7 ^
-Zo ^
-diagnostics:column ^
-fp:fast ^
-nologo ^
-sdl ^
/D_ITERATOR_DEBUG_LEVEL=0

:: Linker flags
:: ---------------------
:: -STACK:reserve[,commit] - Stack size
:: -incremental - Link incrementally
:: -opt:ref - Remove functions and data that are never referenced

set CommonLinkerFlags=/LIBPATH:"C:/opt/lib/" ^
-incremental:no ^
-opt:ref ^
/NODEFAULTLIB:msvcrt.lib ^
opengl32.lib glfw3.lib assimp-vc142-mtd.lib user32.lib gdi32.lib shell32.lib
:: -STACK:0x100000,0x100000

pushd build

cl %CommonCompilerFlags% ..\src\glad.c ..\src\log.cpp ..\src\shader.cpp ..\src\util.cpp ..\src\camera.cpp ..\src\models.cpp ..\src\asset.cpp ..\src\memory.cpp ..\src\gl.cpp /link /SUBSYSTEM:console %CommonLinkerFlags% /out:gl.exe
:: cl %CommonCompilerFlags% ..\src\glad.c ..\src\gl.cpp /link /SUBSYSTEM:console %CommonLinkerFlags% /out:gl.exe

popd
