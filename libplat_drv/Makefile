WORKDIR:=$(shell pwd)
MAKE:=make
CP:=cp -f
RM:=rm -rf

#Make a shared library .so or a static library .a?
COMPILE_SO ?= y


# libs and their include dir
LIB_DIR := $(WORKDIR)/lib
INC_DIR := $(WORKDIR)/include

export LIB_DIR INC_DIR

# compile toolchain
CROSS_COMPILE ?= arm-anykav200-linux-uclibcgnueabi-

DEBUG := n
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++


ifeq ($(DEBUG), y)
	CFLAGS := -O0
	CXXFLAGS := -O0
	STD_LDFLAGS = -lrt -lpthread -ldl -Wl,-rpath=/mnt -rdynamic
	CC += -g
	CXX += -g
else
	CFLAGS := -fno-strict-aliasing -Os
	CXXFLAGS := -fno-strict-aliasing -Os
	STD_LDFLAGS = -lrt -lpthread -ldl -Wl,-rpath=/mnt -rdynamic
endif

CFLAGS += $(addprefix -I,$(INC_DIR))
CFLAGS += -Wall -Werror -D_GNU_SOURCE -std=c99 -fms-extensions
LD:= $(CC)
LDCC := $(CC)
LDCXX := $(CXX)
STRIP :=$(CROSS_COMPILE)strip

AR := $(CROSS_COMPILE)ar
ARFLAGS := crs
export CROSS_COMPILE CC CXX LD LDCC LDCXX STRIP CFLAGS CXXFLAGS AR ARFLAGS STD_LDFLAGS DEBUG


# config compile lib type, build .so default

export PLAT_RELEASE COMPILE_SO LOCAL_COMMON_FLAG LOCAL_GEN COMPILE_DEMO

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
LIB_NAME := libplat_drv

ifeq ($(COMPILE_SO), y)
	LOCAL_COMMON_FLAG := -shared -fPIC -o
	LOCAL_GEN := $(CC)
	TARGET := $(LIB_NAME).so
else
	LOCAL_COMMON_FLAG := $(ARFLAGS)
	LOCAL_GEN := $(AR)
	TARGET := $(LIB_NAME).a
endif

.PHONY: $(TARGET) install

$(TARGET): $(OBJS)
	$(LOCAL_GEN) $(LOCAL_COMMON_FLAG) $@ $^
	$(CP) $(TARGET) $(LIB_DIR)
	@echo ""

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ""
	
clean:	
	$(RM) *.o
	$(RM) $(TARGET)
	$(RM) $(LIB_DIR)/$(TARGET)

install:
	@echo ""

all: 
	make

