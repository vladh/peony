ifeq ($(OS),Windows_NT)
	include make_windows.mk
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		include make_macos.mk
	endif
endif
