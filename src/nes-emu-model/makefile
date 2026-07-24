# ---- Configuration ----
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c99 -g -Iinclude
BIN_DIR := bin
TARGET  := $(BIN_DIR)/app

# ---- Sources ----
SRCS := test.c $(wildcard modules/*.c)

OBJS := $(patsubst %.c,$(BIN_DIR)/%.o,$(SRCS))

# ---- Default target ----
.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/%.o: %.c
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
