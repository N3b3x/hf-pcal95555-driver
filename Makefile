# Simple Makefile for the PCAL95555 driver

CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -I./src

BUILD_DIR := build
LIB := $(BUILD_DIR)/libpcal95555.a
TEST_BIN := $(BUILD_DIR)/test

SRCS := src/pcal95555.cpp
OBJS := $(SRCS:src/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean test

all: $(LIB)

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

test: $(TEST_BIN)

$(TEST_BIN): src/pcal95555_test.cpp $(LIB)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)
