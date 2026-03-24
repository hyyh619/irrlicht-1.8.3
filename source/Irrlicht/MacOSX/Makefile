# Makefile for Irrlicht macOS specific files
# Targets arm64 architecture

CXX = c++
CC = cc

# Paths
IRR_INCLUDE = ../../../include
ZLIB_INCLUDE = ../zlib
LIBPNG_INCLUDE = ../libpng
JPEGLIB_INCLUDE = ../jpeglib

# Flags
# Using the flags requested previously
CXXFLAGS = -Wall -pipe -fno-exceptions -frtti -fstrict-aliasing -g -O0 -D_DEBUG -DGL_SILENCE_DEPRECATION -Wno-deprecated-declarations -arch arm64
CFLAGS = -arch arm64
CPPFLAGS = -I$(IRR_INCLUDE) -I$(ZLIB_INCLUDE) -I$(LIBPNG_INCLUDE) -I$(JPEGLIB_INCLUDE) -I.. -DIRRLICHT_EXPORTS=1

# Source files
SOURCES = AppDelegate.mm CIrrDeviceMacOSX.mm OSXClipboard.mm
OBJECTS = $(SOURCES:.mm=.o)

# Default target
all: $(OBJECTS)

# Rules for Objective-C++ files
%.o: %.mm
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# Clean
clean:
	rm -f $(OBJECTS)

.PHONY: all clean
