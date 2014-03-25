CPP_MODS=../cpp-mods
GRAPPIX=$(CPP_MODS)/grappix
FLATLAND=$(CPP_MODS)/flatland

include $(CPP_MODS)/config.mk

include $(GRAPPIX)/module.mk
include $(FLATLAND)/module.mk
include $(CPP_MODS)/coreutils/module.mk
#include $(CPP_MODS)/lua/module.mk

ifeq ($(HOST),android)
else ifeq ($(HOST),emscripten)
else
include $(CPP_MODS)/backward-cpp/module.mk
CXX=clang++
CC=clang
USE_CCACHE=1
CFLAGS += -Qunused-arguments
endif


CFLAGS += -g -O2 -Wall -Wno-switch 

#USE_CCACHE=1

#PREFIX=ccache

TARGET := rock
LOCAL_FILES := main.cpp Game.cpp

include $(CPP_MODS)/build.mk
