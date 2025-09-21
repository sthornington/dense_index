# Dense Indexed Container - Development Notes

## Project Overview

This is a C++23 header-only library implementing strongly-typed index wrappers for containers. The library prevents index mix-ups at compile time with zero runtime overhead.

## Latest Status (as of last session)

### Recent Major Changes
1. **Removed tag-based backward compatibility** - Container now only accepts strong index types
2. **Simplified API** - One consistent way to use the library
3. **Enhanced flexibility** - Works with any strong type providing `.get()`, `.value()`, or implicit conversion
4. **All tests passing** - Including custom strong type compatibility tests

## Key Implementation Details

### Design Philosophy
- **Type Safety**: Different index domains are completely separate types
- **Zero Runtime Overhead**: Compiles to the same machine code as raw indexing
- **Flexibility**: Works with any strong type that provides value access
- **No Magic**: Explicit strong type usage, no hidden conversions

### Core Components

1. **StrongIndex Template**: Built-in strong type with tag-based differentiation
2. **DenseIndexedContainer**: Container wrapper enforcing strong index types
3. **Concepts**: Extensive use of C++23 concepts for conditional method availability

### Template Parameters

**Important**: `DenseIndexedContainer` is templated on the **strong index type**, not a tag:

```cpp
// Container requires a strong index type that satisfies StrongIndexType concept
DenseIndexedContainer<std::vector<T>, MyStrongIndexType> container;

// Use the built-in StrongIndex template with a tag
struct MyTag {};
using MyIndex = StrongIndex<MyTag>;
DenseIndexedContainer<std::vector<T>, MyIndex> container;
```

This distinction is crucial for understanding what indices mean:
- A `DenseIndexedContainer<vector<EmployeeId>, TeamMemberIndex>` holds employee IDs but is indexed by team position
- The first template parameter is what's stored
- The second template parameter is how it's indexed

### Strong Type Compatibility

The library supports three common strong type access patterns via the `StrongIndexType` concept:

1. **`.get()` method** - Used by NamedType library, std::reference_wrapper, our StrongIndex
2. **`.value()` method** - Used by std::optional and many custom implementations
3. **Implicit conversion** - Used by BOOST_STRONG_TYPEDEF

Priority order in `get_index_value()` helper: `.get()` → `.value()` → implicit conversion

**Important**: Raw integral types (int, size_t) are explicitly excluded from `StrongIndexType` concept

### Key Technical Decisions

#### Current Concept Design (Updated)
```cpp
template<typename T>
concept StrongIndexType =
    !std::is_integral_v<T> &&  // Exclude raw integers
    requires(T t, std::size_t n) {
        { T{n} };  // Constructible from size_t
    } && (
        requires(const T ct) { ct.get(); } ||
        requires(const T ct) { ct.value(); } ||
        requires(const T ct) { static_cast<std::size_t>(ct); }
    );
```

#### Template Design (Simplified - No More Tag Support)
```cpp
template<IndexableContainer Container, StrongIndexType IndexType>
class DenseIndexedContainer {
    // Only accepts types satisfying StrongIndexType concept
};
```

#### Method Constraints
All methods use `requires` clauses for better error messages, even when redundant with class-level constraints.

## File Structure

```
dense_index/
├── dense_index.hpp                    # Main library header
├── test_dense_index.cpp               # Comprehensive test suite
├── test_custom_strong_type.cpp        # Tests with custom strong types
├── test_strong_type_compatibility.cpp # Tests different access patterns (.get/.value/implicit)
├── compile_time_errors.cpp            # Compile-time error validation
├── example.cpp                        # Original usage examples
├── example_modern.cpp                 # Modern API examples with custom strong types
├── README.md                          # User documentation
├── CLAUDE.md                          # This file - development notes
├── Makefile                           # Build configuration
├── .gitignore                         # Excludes build/ directory
└── build/                             # Binary output directory
```

## Build System

- Uses `build/` directory for all binaries
- C++23 with `-Wall -Wextra -Wpedantic`
- Targets: `test`, `test_custom`, `run_example`, `check_errors`

## Testing Strategy

1. **Unit Tests** (`test_dense_index.cpp`)
   - Strong index operations
   - Type safety verification
   - Container operations (vector, array, deque)
   - STL algorithm compatibility
   - Conditional method availability
   - Performance verification (zero overhead)

2. **Custom Type Tests** (`test_custom_strong_type.cpp`)
   - Custom strong types with `.get()`
   - Third-party library simulation
   - Backward compatibility with tags

3. **Compatibility Tests** (`test_strong_type_compatibility.cpp`)
   - BOOST_STRONG_TYPEDEF style (implicit conversion)
   - NamedType style (`.get()`)
   - std::optional style (`.value()`)
   - Mixed access patterns

4. **Compile Error Tests** (`compile_time_errors.cpp`)
   - Wrong index type usage
   - Raw index rejection
   - No implicit conversions
   - Missing container capabilities

## Known Design Trade-offs

1. **Access Method Detection**: Uses compile-time if-constexpr chain in `get_index_value()` rather than specializations for simplicity
2. **Error Messages**: Prioritizes clear error messages over minimal template instantiations
3. **API Simplicity**: Removed tag-based backward compatibility in favor of explicit strong type usage
4. **Performance**: All abstractions compile away - verified by memory comparison test
5. **Integral Type Exclusion**: Explicitly excludes raw integral types from `StrongIndexType` to prevent accidental misuse

## Common Strong Type Libraries Supported

- **BOOST_STRONG_TYPEDEF**: Via implicit conversion operators
- **NamedType** (Jonathan Boccara): Via `.get()` method
- **rollbear/strong_type**: Would need adapter for `value_of()` function
- **type_safe**: Via modular approach
- **Custom implementations**: Any combination of access methods

## Future Considerations

1. Could add support for `value_of()` free function pattern
2. Could provide adapters for specific libraries
3. Could add more sophisticated iterator wrappers that return strong indices
4. Could support strong difference types (not just indices)

## Key Compile Commands

```bash
# Run all tests
make test

# Test custom strong types
make test_custom

# Build and run examples
make run_example

# Verify compile-time errors
make check_errors

# Clean build directory
make clean
```

## Important Implementation Notes

1. **Namespace**: Everything in `dense_index::` namespace except std::hash and std::ranges specializations
2. **Hash Support**: std::hash specialization for StrongIndex types
3. **Range Support**: std::ranges::enable_borrowed_range specialization
4. **No Deduction Guides**: Removed after eliminating tag support
5. **Type Aliases**: DenseVector, DenseArray, DenseDeque now require explicit strong index type
6. **Helper Function**: `get_index_value()` abstracts access pattern differences

## Design Rationale from Original Spec

The implementation follows the design document's goals:
- ✅ Type Safety: Different domains can't be mixed
- ✅ Convenience: Natural API, indices returned from operations
- ✅ Clarity: Error messages show type names
- ✅ Zero Runtime Overhead: Verified via performance test
- ✅ General Applicability: Works with any indexable container
- ✅ Modern C++: Uses concepts, spaceship operator, requires clauses

## Test Coverage Status

All tests passing (verified in latest session):
- Basic strong index operations ✓
- Type safety enforcement ✓
- Container operations (vector, array, deque) ✓
- Iterator utilities ✓
- STL algorithm compatibility ✓
- Conditional method availability ✓
- Construction variants ✓
- Swap functionality ✓
- Comparison operators ✓
- Hash support ✓
- Concept constraints ✓
- Underlying container access ✓
- Type aliases ✓
- Performance (zero overhead) ✓
- Custom strong types ✓
- Multiple access patterns (.get/.value/implicit) ✓
- BOOST_STRONG_TYPEDEF compatibility ✓
- NamedType style compatibility ✓
- std::optional style compatibility ✓

## API Usage Examples

### Basic Usage with Built-in StrongIndex
```cpp
struct EmployeeTag {};
using EmployeeId = StrongIndex<EmployeeTag>;
DenseIndexedContainer<std::vector<Employee>, EmployeeId> employees;
```

### Custom Strong Type
```cpp
class CustomIndex {
    std::size_t value_;
public:
    explicit CustomIndex(std::size_t v) : value_(v) {}
    std::size_t get() const { return value_; }  // or .value() or implicit conversion
    auto operator<=>(const CustomIndex&) const = default;
};
DenseIndexedContainer<std::vector<Data>, CustomIndex> container;
```

### Important Semantic Distinction
```cpp
// Container stores EmployeeId values, indexed by TeamMemberIndex
DenseIndexedContainer<std::vector<EmployeeId>, TeamMemberIndex> team_members;
// team_members[TeamMemberIndex(0)] returns the EmployeeId of the first team member
```