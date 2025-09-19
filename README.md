# Dense Indexed Container Library

A modern C++23 header-only library that provides strongly-typed index wrappers for containers, preventing index mix-ups at compile time with zero runtime overhead.

## Features

- **Type-Safe Indexing**: Different index domains are completely separate types, preventing accidental misuse
- **Zero Runtime Overhead**: Compiles to the same machine code as raw indexing
- **Container Agnostic**: Works with `std::vector`, `std::array`, `std::deque`, and any container with operator[]
- **Conditional Methods**: Only exposes methods that the underlying container supports (via C++20 concepts)
- **STL Compatible**: Works seamlessly with standard algorithms and range-based for loops
- **Clean Error Messages**: Clear compile-time errors when indices are misused

## Quick Example

```cpp
#include "dense_index.hpp"

// Define index domains using tags
struct EmployeeTag {};
struct DepartmentTag {};

using EmployeeIndex = dense_index::StrongIndex<EmployeeTag>;
using DepartmentIndex = dense_index::StrongIndex<DepartmentTag>;

// Create strongly-typed containers with explicit strong index types
dense_index::DenseIndexedContainer<std::vector<std::string>, EmployeeIndex> employees;
dense_index::DenseIndexedContainer<std::vector<std::string>, DepartmentIndex> departments;

// Use the containers
auto emp_idx = employees.push_back("Alice");  // Returns EmployeeIndex
auto dept_idx = departments.push_back("Engineering");  // Returns DepartmentIndex

std::string name = employees[emp_idx];  // OK
// std::string bad = employees[dept_idx];  // Compile error! Wrong index type
// std::string bad2 = employees[0];  // Compile error! Raw indices not allowed
```

## Requirements

- C++23 compatible compiler (tested with GCC 13+, Clang 16+)
- Standard library with C++23 support

## Installation

Simply copy `dense_index.hpp` to your project and include it:

```cpp
#include "dense_index.hpp"
```

## Building Tests and Examples

```bash
# Build and run all tests
make test

# Build and run examples
make run_example

# Check compile-time error detection
make check_errors

# Build everything
make all
```

## Design Philosophy

The library follows these principles:

1. **Compile-Time Safety**: All index type mismatches are caught at compile time
2. **Zero Overhead**: No runtime cost compared to raw indexing
3. **Ergonomic API**: Natural to use, with minimal boilerplate
4. **Standard Compatibility**: Works with STL algorithms and idioms
5. **Concept-Based**: Uses C++20 concepts for clean, conditional APIs

## API Overview

### Strong Index Type

```cpp
template<IndexTag Tag>
class StrongIndex {
    // Construction
    explicit StrongIndex(size_t value);

    // Access
    size_t value() const;
    explicit operator size_t() const;

    // Comparisons (spaceship operator)
    auto operator<=>(const StrongIndex&) const = default;

    // Arithmetic
    StrongIndex operator+(size_t) const;
    StrongIndex operator-(size_t) const;
    ptrdiff_t operator-(StrongIndex) const;

    // Increment/decrement
    StrongIndex& operator++();
    StrongIndex operator++(int);
};
```

### Dense Indexed Container

```cpp
template<IndexableContainer Container, IndexTag Tag>
class DenseIndexedContainer {
    // Element access (only with strong indices)
    reference operator[](index_type);
    reference at(index_type);

    // Modifiers that return indices
    index_type push_back(const value_type&);
    index_type emplace_back(Args&&...);
    index_type insert(index_type pos, const value_type&);
    index_type erase(index_type pos);

    // Standard container interface
    iterator begin();
    iterator end();
    size_type size() const;
    bool empty() const;

    // Utility
    index_type index_of(const_iterator);
    iterator iterator_at(index_type);
    Container& underlying();
};
```

## Advanced Usage

### Custom Container Support

Any container that provides `operator[]`, `size()`, and iterators works:

```cpp
template<typename T>
struct MyContainer {
    T& operator[](size_t);
    size_t size() const;
    // ... iterators, etc.
};

dense_index::DenseIndexedContainer<MyContainer<int>, MyTag> my_container;
```

### Conditional Methods

Methods are only available if the underlying container supports them:

```cpp
// Vector has push_back, capacity, reserve
dense_index::DenseVector<int, VecTag> vec;
vec.push_back(42);  // OK
vec.reserve(100);   // OK

// Array doesn't have push_back
dense_index::DenseArray<int, 10, ArrTag> arr;
// arr.push_back(42);  // Compile error: no push_back
```

### Type Aliases

Convenient aliases for common containers:

```cpp
template<typename T, typename Tag = struct DefaultIndexTag>
using DenseVector = DenseIndexedContainer<std::vector<T>, Tag>;

template<typename T, size_t N, typename Tag = struct DefaultIndexTag>
using DenseArray = DenseIndexedContainer<std::array<T, N>, Tag>;

template<typename T, typename Tag = struct DefaultIndexTag>
using DenseDeque = DenseIndexedContainer<std::deque<T>, Tag>;
```

## Performance

The library has zero runtime overhead. The strong index types compile to simple integers, and all wrapper methods are inlined. See `test_performance()` in the test suite for verification.

## Error Messages

The library provides clear compile-time errors:

```cpp
employees[dept_idx];
// error: no match for 'operator[]' (operand types are
// 'DenseIndexedContainer<vector<string>, EmployeeTag>' and 'DepartmentIndex')

employees[0];
// error: use of deleted function 'operator[](size_type)'
```

## Testing

The library includes comprehensive tests:
- Unit tests for all functionality (`test_dense_index.cpp`)
- Compile-time error tests (`compile_time_errors.cpp`)
- Practical usage examples (`example.cpp`)

## License

This implementation follows the design specification provided and is intended for educational and practical use.

## Contributing

Contributions are welcome! Please ensure all tests pass and add new tests for new features.