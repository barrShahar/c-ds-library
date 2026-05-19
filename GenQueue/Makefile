# Compiler settings
CC = gcc
CFLAGS = -g -pedantic -Wall -Wextra  #-I./include

# Directories
SRC_DIR = .
TEST_DIR = tests

# Executable names
APP_BIN = run

TEST_GENQUEUE_BIN = test_q


# Default target: builds both the app and the test runner
all: $(TEST_GENQUEUE_BIN)


# Target 4: Build the genqueue test runner
$(TEST_GENQUEUE_BIN): $(TEST_DIR)/test_genqueue.c $(SRC_DIR)/genqueue.c 
	$(CC) $(CFLAGS) $(TEST_DIR)/test_genqueue.c $(SRC_DIR)/genqueue.c -o $(TEST_GENQUEUE_BIN)

# Target 5: Clean up compiled files
clean:
	rm -f $(TEST_GENQUEUE_BIN)
