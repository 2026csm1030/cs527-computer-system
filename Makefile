# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Output executable name
TARGET = mini_computer

# Source files and object files
SRCS = main.c compiler.c memory.c processor.c
OBJS = $(SRCS:.c=.o)

# Default test case directory
TEST_DIR ?= tests/test1_sum

# Default rule: build executable
all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile individual C source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the simulation using files from a specified TEST_DIR
run: $(TARGET)
	@cp $(TEST_DIR)/input.txt ./input.txt
	@cp $(TEST_DIR)/data.byte ./data.byte
	@echo "--- Running test from $(TEST_DIR) ---"

# Preset targets for individual tests
test-sum:
	$(MAKE) run TEST_DIR=tests/test1_sum

test-complex:
	$(MAKE) run TEST_DIR=tests/test2_complex

test-matrix:
	$(MAKE) run TEST_DIR=tests/test3_matrix

test-runtime-sum:
	$(MAKE) run TEST_DIR=tests/test4_runtime_sum

test-fir:
	$(MAKE) run TEST_DIR=tests/test5_fir_filter

# Clean up build artifacts and output files
clean:
	rm -f $(OBJS) $(TARGET) program.byte data.byte input.txt

.PHONY: all clean run test-sum test-complex test-matrix test-runtime-sum test-fir-filter