# ---- Configuration ----
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c99 -g -Iinclude
BIN_DIR := bin
TARGET  := $(BIN_DIR)/app

# ---- Sources ----
SRCS := src/main.c $(wildcard src/modules/*.c)

# Map each .c to a .o inside bin/, mirroring the src/ structure
# e.g. src/main.c            -> bin/src/main.o
#      src/modules/foo.c     -> bin/src/modules/foo.o
OBJS := $(patsubst src/%.c,$(BIN_DIR)/src/%.o,$(SRCS))

# ---- Default target ----
.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Pattern rule to compile any src/*.c into bin/src/*.o, creating dirs as needed
$(BIN_DIR)/src/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ---- Run ----
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# ---- Clean ----
.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
