# Dense Indexed Container - Development Notes

## Project Overview

This is a C++23 header-only library implementing strongly-typed index wrappers for containers. The library prevents index mix-ups at compile time with zero runtime overhead.

## Key Implementation Details

### Design Philosophy
- **Type Safety**: Different index domains are completely separate types
- **Zero Runtime Overhead**: Compiles to the same machine code as raw indexing
- **Flexibility**: Works with any strong type that provides value access
- **Backward Compatibility**: Maintains support for simple tag-based interface

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

The library supports three common strong type access patterns:

1. **`.get()` method** - Used by NamedType library, std::reference_wrapper
2. **`.value()` method** - Used by std::optional and many custom implementations
3. **Implicit conversion** - Used by BOOST_STRONG_TYPEDEF

Priority order: `.get()` → `.value()` → implicit conversion

### Key Technical Decisions

#### Concept Design
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

#### Unified Template
Single template handles both tag types and strong index types:
- If `IndexTag` → wraps with `StrongIndex<Tag>`
- If `StrongIndexType` → uses directly

#### Method Constraints
All methods use `requires` clauses for better error messages, even when redundant with class-level constraints.

## File Structure

```
dense_index/
├── dense_index.hpp                    # Main library header
├── test_dense_index.cpp               # Comprehensive test suite
├── test_custom_strong_type.cpp        # Tests with custom strong types
├── test_strong_type_compatibility.cpp # Tests different access patterns
├── compile_time_errors.cpp            # Compile-time error validation
├── example.cpp                        # Usage examples
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

1. **Access Method Detection**: Uses compile-time if-constexpr chain rather than specializations for simplicity
2. **Error Messages**: Prioritizes clear error messages over minimal template instantiations
3. **Backward Compatibility**: Maintains tag-based interface using conditional compilation
4. **Performance**: All abstractions compile away - verified by memory comparison test

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

1. **Namespace**: Everything in `dense_index::` namespace except std::hash specialization
2. **Hash Support**: std::hash specialization for StrongIndex types
3. **Range Support**: std::ranges::enable_borrowed_range specialization
4. **Deduction Guides**: Automatic selection between tag and strong type modes
5. **Type Aliases**: DenseVector, DenseArray, DenseDeque for convenience

## Design Rationale from Original Spec

The implementation follows the design document's goals:
- ✅ Type Safety: Different domains can't be mixed
- ✅ Convenience: Natural API, indices returned from operations
- ✅ Clarity: Error messages show type names
- ✅ Zero Runtime Overhead: Verified via performance test
- ✅ General Applicability: Works with any indexable container
- ✅ Modern C++: Uses concepts, spaceship operator, requires clauses

## Test Coverage Status

All tests passing:
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
- Multiple access patterns ✓
- Backward compatibility ✓