FLAGS = -Wall -Wextra -pedantic -g
CC = gcc

TEST_DIR = tests
BIN_DIR = bin
$(shell mkdir -p $(BIN_DIR))

TARGET = $(BIN_DIR)/test_dlist

SRC_OBJS = $(BIN_DIR)/gen_dlist.o
TEST_OBJS = $(BIN_DIR)/test_gen_dlist.o


$(TARGET): $(SRC_OBJS) $(TEST_OBJS)
	$(CC) $(FLAGS) -o $@ $^

$(BIN_DIR)/%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJS) $(TEST_OBJS) $(TARGET)

.PHONY: clean
