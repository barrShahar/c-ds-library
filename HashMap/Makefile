FLAGS = -Wall -Wextra -pedantic -g
CC = gcc

TEST_DIR = tests
BIN_DIR = bin
DLL_DIR = ../DoubleLinkedList

TARGET = $(BIN_DIR)/test_hashmap

SRC_OBJS = $(BIN_DIR)/HashMap.o $(BIN_DIR)/gen_dlist.o
TEST_OBJS = $(BIN_DIR)/TestHashMap.o


$(TARGET): $(SRC_OBJS) $(TEST_OBJS)
	$(CC) $(FLAGS) -o $@ $^

# Ensure bin/ exists before compiling
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/%.o: %.c | $(BIN_DIR)
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(TEST_DIR)/%.c | $(BIN_DIR)
	$(CC) $(FLAGS) -c $< -o $@

# Double linked list directory
$(BIN_DIR)/%.o: $(DLL_DIR)/%.c | $(BIN_DIR)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJS) $(TEST_OBJS) $(TARGET)

.PHONY: clean