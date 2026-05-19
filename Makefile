FLAGS = -Wall -Wextra -pedantic # -g
CC = gcc
AR = ar cr
PROJECT_NAME = DataStructures

VECTOR_DIR = GenVector
HASH_MAP_DIR = HashMap
BIN_TREE_DIR = BinTree
GEN_HEAP_DIR = GenHeap
DLIST_DIR = DoubleLinkedList
QUEUE_DIR = GenQueue
BIN_DIR = bin
LIB_DIR = lib
$(shell mkdir -p $(BIN_DIR))
$(shell mkdir -p $(LIB_DIR))

# Creating a static library for the project
TARGET_STATIC_LIB = $(LIB_DIR)/lib$(PROJECT_NAME).a

ALL_OBJS = $(BIN_DIR)/vector.o $(BIN_DIR)/HashMap.o $(BIN_DIR)/bin_tree.o $(BIN_DIR)/genHeap.o $(BIN_DIR)/gen_dlist.o $(BIN_DIR)/genqueue.o

all: $(TARGET_STATIC_LIB)

$(TARGET_STATIC_LIB): $(ALL_OBJS) 
	$(AR) $@ $^

# Create bin directory if not exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Create lib directory if not exists
$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(BIN_DIR)/%.o: $(VECTOR_DIR)/%.c 
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(HASH_MAP_DIR)/%.c 
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(BIN_TREE_DIR)/%.c 
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(GEN_HEAP_DIR)/%.c 
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(DLIST_DIR)/%.c 
	$(CC) $(FLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(QUEUE_DIR)/%.c 
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR) $(LIB_DIR)

.PHONY: all clean



