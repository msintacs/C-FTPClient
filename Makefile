CC = gcc

ifeq ($(OS),Windows_NT)
	TARGET = main.exe
	BUILD_DIR = build
	RMDIR = rmdir /s /q
	RMFILE = del /f /q
	MKDIR = if not exist $(subst /,\,$(@D)) mkdir $(subst /,\,$(@D))	
	SRCS = $(wildcard $(SRC_ROOT)/*.c) $(wildcard $(SRC_ROOT)/*/*.c)
	INCLUDE_DIRS = $(wildcard ./include/*/)
	LIBS = -lws2_32
else
	TARGET = main.out
	BUILD_DIR = build
	RMDIR = rm -rf
	RMFILE = rm -rf 
	MKDIR = mkdir -p $(@D)
	SRCS = $(shell find $(SRC_ROOT) -name '*.c')
	INCLUDE_DIRS = $(shell find ./include -type d)
	LIBS =
endif

CFLAGS = -Wall -Wextra -O2

SRC_ROOT = ./src
INCLUDE_DIR = $(foreach dir,$(INCLUDE_DIRS),-I$(dir))
OBJS = $(SRCS:$(SRC_ROOT)/%.c=$(BUILD_DIR)/%.o)

.PHONY : all clean

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

$(BUILD_DIR)/%.o : $(SRC_ROOT)/%.c
	@$(MKDIR)
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c $< -o $@

clean :
	$(RMDIR) $(BUILD_DIR)
	$(RMFILE) $(TARGET)