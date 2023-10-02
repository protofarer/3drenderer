CC := gcc
CC_FLAGS := -Wall
LANG_STD = -std=c99
# INCLUDE_PATH := -I"./libs"
SRCS := src/*.c
LINKER_FLAGS := -lSDL2 -lm
EXECUTABLE = renderer
# OBJS := $(patsubst %.c, %.o, $(SRCS))

build:
	$(CC) $(CC_FLAGS) $(LANG_STD) $(SRCS) $(LINKER_FLAGS) -o $(EXECUTABLE)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: clean
clean:
	rm ./renderer
	# rm -f $(EXECUTABLE) $(OBJS)