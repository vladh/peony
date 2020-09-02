@echo off

:: -FC - Display full path of source code files in warnings
:: -GL - Enable whole program optimisation
:: -GS - Add debugging info to object files
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

set CommonLinkerFlags=/LIBPATH:"C:/opt/lib/" -STACK:0x100000,0x100000 -incremental:no -opt:ref /NODEFAULTLIB:msvcrt.lib opengl32.lib glfw3.lib assimp-vc142-mtd.lib user32.lib gdi32.lib shell32.lib

pushd build

cl %CommonCompilerFlags% -MDd ..\src\glad.c ..\src\log.cpp ..\src\shader.cpp ..\src\util.cpp ..\src\camera.cpp ..\src\models.cpp ..\src\gl.cpp /link /SUBSYSTEM:console %CommonLinkerFlags% /out:gl.exe

popd
