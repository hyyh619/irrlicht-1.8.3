#! /bin/sh
#
# MacOS_build.sh
# Copyright (C) 2026 yinghuang <yinghuang@MacMiniM4Pro>
#
# Distributed under terms of the MIT license.
#


make staticlib_osx -j8 && ar rs libIrrlicht.a MacOSX/CIrrDeviceMacOSX.o MacOSX/OSXClipboard.o MacOSX/AppDelegate.o && cp libIrrlicht.a ../../lib/MacOSX/libIrrlicht.a
