@echo off

:: set CommonCompilerFlags=/I "C:/opt/include/" -diagnostics:column -WL -Od -nologo -fp:fast -Zo -Oi -W4 -FC -Z7 /D_ITERATOR_DEBUG_LEVEL=0
set CommonCompilerFlags=/I "C:/opt/include/" -diagnostics:column -WL -nologo -fp:fast -Zo -Oi -W4 -FC -Z7 /D_ITERATOR_DEBUG_LEVEL=0
set CommonLinkerFlags=/LIBPATH:"C:/opt/lib/" -STACK:0x100000,0x100000 -incremental:no -opt:ref /NODEFAULTLIB:msvcrt.lib opengl32.lib glfw3.lib assimp-vc142-mtd.lib user32.lib gdi32.lib shell32.lib

pushd build

cl %CommonCompilerFlags% -MDd ..\src\glad.c ..\src\log.cpp ..\src\shader.cpp ..\src\util.cpp ..\src\camera.cpp ..\src\models.cpp ..\src\gl.cpp /link /SUBSYSTEM:console %CommonLinkerFlags% /out:gl.exe

popd
