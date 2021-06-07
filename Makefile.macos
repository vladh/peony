COMPILER_FLAGS =\
	-ggdb3 -D_FORTIFY_SOURCE=2 -Og -Wall -Werror -Wno-deprecated-volatile -pedantic\
	-Wno-unused-function -Wno-unknown-pragmas -Wno-comment\
	-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX

LINKER_FLAGS =\
	-lcrt -lc

.PHONY: unity run

unity:
	time g++ $(COMPILER_FLAGS) $(LINKER_FLAGS) src/_unity.cpp -o bin/peony

run:
	./bin/peony
