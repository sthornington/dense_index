CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic -O2 -march=native
DEBUG_FLAGS = -g -O0 -fsanitize=address -fsanitize=undefined
TEST_FLAGS = -std=c++23 -Wall -Wextra -Wpedantic

# Output directory
BUILD_DIR = build

# Targets
TARGETS = $(BUILD_DIR)/test_dense_index $(BUILD_DIR)/example $(BUILD_DIR)/test_custom_strong_type

.PHONY: all clean test debug run_example check_errors

all: $(TARGETS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/test_dense_index: test_dense_index.cpp dense_index.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/example: example.cpp dense_index.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_custom_strong_type: test_custom_strong_type.cpp dense_index.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

debug: test_dense_index.cpp dense_index.hpp | $(BUILD_DIR)
	$(CXX) $(TEST_FLAGS) $(DEBUG_FLAGS) -o $(BUILD_DIR)/test_dense_index_debug test_dense_index.cpp

test: $(BUILD_DIR)/test_dense_index
	$(BUILD_DIR)/test_dense_index

test_custom: $(BUILD_DIR)/test_custom_strong_type
	$(BUILD_DIR)/test_custom_strong_type

run_example: $(BUILD_DIR)/example
	$(BUILD_DIR)/example

# Check that compile-time errors work as expected (should fail to compile)
check_errors: compile_time_errors.cpp dense_index.hpp
	@echo "Testing compile-time error detection..."
	@echo "The following tests should FAIL to compile when uncommented:"
	@echo "  - Wrong index type usage"
	@echo "  - Raw index usage"
	@echo "  - Index type mixing"
	@echo "  - Implicit conversions"
	@echo "Compiling with all errors commented (should succeed)..."
	$(CXX) $(TEST_FLAGS) -c compile_time_errors.cpp -o /dev/null
	@echo "✓ Compile-time error test file is valid"

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Available targets:"
	@echo "  make all          - Build all examples and tests"
	@echo "  make test         - Build and run tests"
	@echo "  make run_example  - Build and run usage examples"
	@echo "  make debug        - Build tests with debug symbols and sanitizers"
	@echo "  make check_errors - Verify compile-time error detection"
	@echo "  make clean        - Remove built files"