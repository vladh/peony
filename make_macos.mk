# (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

COMPILER_FLAGS = \
	-I/usr/local/opt/glm/include \
	-I/usr/local/opt/glfw/include \
	-I/usr/local/opt/assimp/include \
	-I/usr/local/opt/freetype/include/freetype2 \
	-D_FORTIFY_SOURCE=2 -ggdb3 -Og -Wall -Werror -Wextra -pedantic \
	-std=c++2a \
	-Wno-deprecated-volatile -Wno-unused-function -Wno-unknown-pragmas -Wno-comment \
	-Wno-unused-parameter -Wno-sign-compare

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
	@./bin/peony.app/Contents/MacOS/peony
