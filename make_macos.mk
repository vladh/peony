# Peony Game Engine
# Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
# All rights reserved.

COMPILER_FLAGS = \
	-I/usr/local/opt/glm/include \
	-I/usr/local/opt/glfw/include \
	-I/usr/local/opt/assimp/include \
	-I/usr/local/opt/freetype/include/freetype2 \
	-std=c++2a \
	-ggdb3 -D_FORTIFY_SOURCE=2 -Og -Wall -Werror -Wno-deprecated-volatile -pedantic \
	-Wno-unused-function -Wno-unknown-pragmas -Wno-comment \
	-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX

LINKER_FLAGS = \
	-L/usr/local/opt/glfw/lib \
	-L/usr/local/opt/assimp/lib \
	-L/usr/local/opt/freetype/lib \
	-lfreetype -lglfw -lassimp -lm

.PHONY: unity unity-bundle run

unity-bundle: unity
	mkdir -p bin/peony.app/Contents/MacOS
	cp bin/peony bin/peony.app/Contents/MacOS/
	cp extra/Info.plist bin/peony.app/Contents/

unity:
	@echo "################################################################################"
	@echo "### Building"
	@echo "################################################################################"
	time g++ $(COMPILER_FLAGS) $(LINKER_FLAGS) src/_unity.cpp -o bin/peony

run:
	# ./bin/peony
	./bin/peony.app/Contents/MacOS/peony
