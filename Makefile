CC=gcc
PROGNAME?=FlappyBirby
LIB_DIR?=./lib
BUILD_DIR?=./build
SRC_DIRS?=./src
CFLAGS=-O2 -g -Wall -W -std=gnu89 -Wno-unused-parameter
LDFLAGS=-lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_image -lm
SRCS:=$(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS:=$(SRCS:%=$(BUILD_DIR)/%.o)
DEPS:=$(OBJS:.o=.d)
INC_DIRS:=$(shell find $(SRC_DIRS) -type d)
INC_FLAGS:=$(addprefix -I,$(INC_DIRS))
CPPFFLAGS?=$(INC_FLAGS) -MMD -MP

ASSETS_DIR=./assets

$(BUILD_DIR)/$(PROGNAME): $(OBJS)
	$(CC) $(OBJS) -o $@-debug $(LDFLAGS)
	$(CC) $(OBJS) -o $@-release $(LDFLAGS)
	$(CP_R) $(ASSETS_DIR) $(BUILD_DIR)

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
MKDIR_P?=mkdir -p
MV_R?=mv -r
CP_R?=cp -r
