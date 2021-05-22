ifeq ($(OS),Windows_NT)
	CFLAGS = \
	-I "C:/local/include/" \
	-FC \
	-GS \
	-MTd \
	-MP18 \
	-W4 \
	-WL \
	-WX \
	-Z7 \
	-Zo \
	-diagnostics:column \
	-fp:fast \
	-nologo \
	-sdl \
	-guard:cf \
	-permissive- \
	-std:c++latest \
	-wd4100 -wd4127 -wd4201 \
	-O2 \
	-GL \
	-Oi

	LFLAGS = \
	-LIBPATH:"C:/local/lib/" \
	-CGTHREADS:8 \
	-DEBUG:FULL \
	-INCREMENTAL:NO \
	-opt:ref \
	-NODEFAULTLIB:msvcrt.lib \
	freetype.lib opengl32.lib glfw3.lib assimp-vc142-mtd.lib user32.lib gdi32.lib shell32.lib

	TARGET = src/peony.cc
	BINARY = bin/peony.exe
endif

.PHONY: default

default:
	@ if [ -z "${TARGET}" ]; then echo "Unsupported OS"; exit 1; fi
	@ if [ -z "${DevEnvDir+x}" ]; then vcvarsall x64; fi
	cl ${CFLAGS} ${TARGET} -link -SUBSYSTEM:console ${LFLAGS} -out:${BINARY}
