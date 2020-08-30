@echo off

set CommonCompilerFlags=/I "C:/opt/include/" -diagnostics:column -WL -Od -nologo -fp:fast -Zo -Oi -WX -W4 -FC -Z7 -EHscc
set CommonLinkerFlags=/LIBPATH:"C:/opt/lib/" -STACK:0x100000,0x100000 -incremental:no -opt:ref /NODEFAULTLIB:msvcrt.lib opengl32.lib glfw3.lib user32.lib gdi32.lib shell32.lib

pushd build

cl %CommonCompilerFlags% -MDd ..\src\glad.c ..\src\gl.cpp /link /SUBSYSTEM:console %CommonLinkerFlags% /out:gl.exe

popd
