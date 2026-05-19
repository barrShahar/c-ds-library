FLAGS = -Wall -Wextra -pedantic -g
CC = gcc

TEST_DIR = tests
BIN_DIR = bin
VEC_DIR = ../GenVector

TARGET = $(BIN_DIR)/test_heap

SRC_OBJS = $(BIN_DIR)/genHeap.o $(BIN_DIR)/vector.o
TEST_OBJS = $(BIN_DIR)/test_genHeap.o


$(TARGET): $(SRC_OBJS) $(TEST_OBJS) | $(BIN_DIR)
	$(CC) $(FLAGS) -o $@ $^

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/%.o: %.c | $(BIN_DIR)
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(TEST_DIR)/%.c | $(BIN_DIR)
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(VEC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJS) $(TEST_OBJS) $(TARGET)

.PHONY: clean