# Peony Game Engine
# Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
# All rights reserved.

COMPILER_FLAGS = \
	-I/usr/include/glm/include \
	-I/usr/include/GLFW/include \
	-I/usr/include/assimp/include \
	-I/usr/include/freetype2 \
	-D_FORTIFY_SOURCE=2 -ggdb3 -Og -Wall -Werror -Wextra -pedantic \
	-std=c++2a \
	-Wno-deprecated-volatile -Wno-unused-function -Wno-unknown-pragmas -Wno-comment \
	-Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers \
	-Wno-unused-result -Wno-class-memaccess -Wno-unused-but-set-variable

LINKER_FLAGS = \
	-L/usr/lib/x86_64-linux-gnu \
	-lfreetype -lglfw -lassimp -lm -ldl -pthread

.PHONY: unity run

unity:
	@echo "################################################################################"
	@echo "### Building"
	@echo "################################################################################"
	time g++ $(COMPILER_FLAGS) src/_unity.cpp -o bin/peony $(LINKER_FLAGS)

run:
	@./bin/peony
