# (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

COMPILER_FLAGS = \
	 $(shell pkg-config --cflags freetype2)
	-D_FORTIFY_SOURCE=2 -ggdb3 -Og -Wall -Werror -Wextra -pedantic \
	-std=c++2a \
	-Wno-deprecated-volatile -Wno-unused-function -Wno-unknown-pragmas -Wno-comment \
	-Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers \
	-Wno-unused-result -Wno-class-memaccess -Wno-unused-but-set-variable

LINKER_FLAGS = -lfreetype -lglfw -lassimp -lm -ldl -pthread

.PHONY: unity run

unity:
	@echo "################################################################################"
	@echo "### Building"
	@echo "################################################################################"
	time g++ $(COMPILER_FLAGS) src/_unity.cpp -o bin/peony $(LINKER_FLAGS)

run:
	@./bin/peony
