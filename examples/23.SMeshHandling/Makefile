# Makefile for Irrlicht Examples
# It's usually sufficient to change just the target name and source file list
# and be sure that CXX is set to a valid compiler
Target = 23.SMeshHandling
IrrlichtHome = ../..

Sources = main.cpp

# general compiler settings
CPPFLAGS = -I../../include -I/usr/X11R6/include
CXXFLAGS = -g -O0 -ffast-math
#CXXFLAGS = -g -Wall

#default target is Linux
all: all_linux
# macOS target
all_macos clean_macos: SYSTEM=MacOSX
all_macos clean_macos: CXXFLAGS += -arch arm64
all_macos clean_macos: CFLAGS += -arch arm64
all_macos clean_macos: LDFLAGS += -arch arm64

all_macos clean_macos: LDFLAGS += -L$(IrrlichtHome)/lib/$(SYSTEM) -lIrrlicht -framework Cocoa -framework OpenGL -framework IOKit -framework Carbon -framework CoreGraphics
all_macos:
	$(warning Building for Mac...)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(Sources) -o $(DESTPATH) $(LDFLAGS) && dsymutil $(DESTPATH)


ifeq ($(HOSTTYPE), x86_64)
LIBSELECT=64
endif

# target specific settings
all_linux: LDFLAGS = -L/usr/X11R6/lib$(LIBSELECT) -L../../lib/Linux -lIrrlicht -lGL -lXxf86vm -lXext -lX11 -lXcursor
all_linux clean_linux: SYSTEM=Linux
all_win32: LDFLAGS = -L../../lib/Win32-gcc -lIrrlicht -lopengl32 -lm
all_win32 clean_win32: SYSTEM=Win32-gcc
all_win32 clean_win32: SUF=.exe
# name of the binary - only valid for targets which set SYSTEM
DESTPATH = ../../bin/$(SYSTEM)/$(Target)$(SUF)

all_linux all_win32:
	$(warning Building...)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(Sources) -o $(DESTPATH) $(LDFLAGS) && dsymutil $(DESTPATH)

clean: clean_linux clean_win32
	$(warning Cleaning...)

clean_linux clean_win32:
	@$(RM) $(DESTPATH)

.PHONY: all all_win32 clean clean_linux clean_win32
