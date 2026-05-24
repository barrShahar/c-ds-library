FLAGS = -Wall -Wextra -pedantic -g
CC = gcc

TEST_DIR = tests
BIN_DIR = bin
VECTOR_DIR = ../GenVector

TARGET = $(BIN_DIR)/test_stack

SRC_OBJS = $(BIN_DIR)/vector.o $(BIN_DIR)/genstack.o
TEST_OBJS = $(BIN_DIR)/test_stack.o

$(shell mkdir -p $(BIN_DIR))


$(TARGET): $(SRC_OBJS) $(TEST_OBJS)
	$(CC) $(FLAGS) -o $@ $^

$(BIN_DIR)/%.o: %.c 
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(TEST_DIR)/%.c 
	$(CC) $(FLAGS) -c $< -o $@

# Compiling from the GenVector directory
$(BIN_DIR)/%.o: $(VECTOR_DIR)/%.c 
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJS) $(TEST_OBJS) $(TARGET)

.PHONY: clean