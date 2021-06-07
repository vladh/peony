ifeq ($(OS),Windows_NT)
	include Makefile.windows
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		include Makefile.macos
	endif
endif
