CC := gcc
FLAGS := -I./headers -Wall -Wextra -MMD -MP

SRC_DIR := ./src
OBJ_DIR := ./obj
TRNSLTR_SRC_DIR := ./translate-tool

LIBS := -lncurses

SRCS := $(wildcard $(SRC_DIR)/*.c)
TRNSLTR_SRCS := $(wildcard $(TRNSLTR_SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TRNSLTR_OBJS := $(TRNSLTR_SRCS:$(TRNSLTR_SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET := drawing-tui
TEST_TARGET:= tuiTest
TRNSLTR_TARGET:= translator

.PHONY: test build-translator

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) $^ -o $@ $(LIBS)

$(TEST_TARGET): $(OBJS)
	$(CC) $(FLAGS) $^ -o $@ $(LIBS)

$(TRNSLTR_TARGET): $(filter-out %drawing-tui.o, $(OBJS)) $(TRNSLTR_OBJS)
	$(CC) $(FLAGS) $^ -o $@ $(LIBS)

$(OBJ_DIR):
	mkdir ./obj

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TRNSLTR_SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

build: $(TARGET)

build-test: $(TEST_TARGET)

test-run:
	./$(TEST_TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TEST_TARGET) $(TRNSLTR_TARGET)

clean-build: clean build
test-clean-build: clean build-test

test: build-test test-run

build-translator: $(TRNSLTR_TARGET)

-include $(OBJS:.o=.d)
