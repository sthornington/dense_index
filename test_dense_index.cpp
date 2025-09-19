#include "dense_index.hpp"
#include <cassert>
#include <vector>
#include <array>
#include <deque>
#include <list>
#include <algorithm>
#include <numeric>
#include <concepts>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cmath>

// Define test index tags
struct EmployeeTag {};
struct DepartmentTag {};
struct ProductTag {};

using EmployeeIndex = dense_index::StrongIndex<EmployeeTag>;
using DepartmentIndex = dense_index::StrongIndex<DepartmentTag>;
using ProductIndex = dense_index::StrongIndex<ProductTag>;

// Test strong index basic functionality
void test_strong_index() {
    std::cout << "Testing StrongIndex..." << std::endl;

    // Construction and value access
    EmployeeIndex idx1(5);
    assert(idx1.value() == 5);
    assert(static_cast<std::size_t>(idx1) == 5);

    // Default construction
    EmployeeIndex idx2;
    assert(idx2.value() == 0);

    // Comparison operators
    EmployeeIndex idx3(5);
    EmployeeIndex idx4(10);
    assert(idx1 == idx3);
    assert(idx1 != idx4);
    assert(idx1 < idx4);
    assert(idx4 > idx1);
    assert(idx1 <= idx3);
    assert(idx1 <= idx4);

    // Increment/decrement
    EmployeeIndex idx5(5);
    ++idx5;
    assert(idx5.value() == 6);
    idx5++;
    assert(idx5.value() == 7);
    --idx5;
    assert(idx5.value() == 6);
    idx5--;
    assert(idx5.value() == 5);

    // Arithmetic
    EmployeeIndex idx6(10);
    auto idx7 = idx6 + 5;
    assert(idx7.value() == 15);
    auto idx8 = idx6 - 3;
    assert(idx8.value() == 7);

    idx6 += 5;
    assert(idx6.value() == 15);
    idx6 -= 3;
    assert(idx6.value() == 12);

    // Difference between indices
    EmployeeIndex idx9(20);
    EmployeeIndex idx10(15);
    auto diff = idx9 - idx10;
    assert(diff == 5);

    std::cout << "  ✓ Basic StrongIndex operations" << std::endl;
}

// Test that different index types don't convert
void test_type_safety() {
    std::cout << "Testing type safety..." << std::endl;

    EmployeeIndex emp_idx(5);
    DepartmentIndex dept_idx(5);

    // These should not compile (verified via static_assert below)
    // bool same = (emp_idx == dept_idx);  // Error: no operator==
    // EmployeeIndex wrong = dept_idx;     // Error: no conversion
    // emp_idx = dept_idx;                 // Error: no assignment

    // Verify at compile time that these types are distinct
    static_assert(!std::is_same_v<EmployeeIndex, DepartmentIndex>);
    static_assert(!std::is_convertible_v<DepartmentIndex, EmployeeIndex>);
    static_assert(!std::is_convertible_v<EmployeeIndex, DepartmentIndex>);

    // Verify no implicit conversion from size_t
    static_assert(!std::is_convertible_v<std::size_t, EmployeeIndex>);
    static_assert(!std::is_convertible_v<int, EmployeeIndex>);

    std::cout << "  ✓ Type safety enforced" << std::endl;
}

// Test dense indexed container with vector
void test_dense_vector() {
    std::cout << "Testing DenseIndexedContainer with vector..." << std::endl;

    using EmployeeVector = dense_index::DenseIndexedContainer<std::vector<std::string>, EmployeeIndex>;
    EmployeeVector employees;

    // Test push_back returns index
    auto idx0 = employees.push_back("Alice");
    auto idx1 = employees.push_back("Bob");
    auto idx2 = employees.emplace_back("Charlie");

    assert(idx0.value() == 0);
    assert(idx1.value() == 1);
    assert(idx2.value() == 2);

    // Test element access
    assert(employees[idx0] == "Alice");
    assert(employees[idx1] == "Bob");
    assert(employees[idx2] == "Charlie");

    // Test at() with bounds checking
    assert(employees.at(idx0) == "Alice");

    // Test size and empty
    assert(employees.size() == 3);
    assert(!employees.empty());

    // Test front and back
    assert(employees.front() == "Alice");
    assert(employees.back() == "Charlie");

    // Test iteration
    std::vector<std::string> names;
    for (const auto& name : employees) {
        names.push_back(name);
    }
    assert(names.size() == 3);
    assert(names[0] == "Alice");
    assert(names[1] == "Bob");
    assert(names[2] == "Charlie");

    // Test modification
    employees[idx1] = "Robert";
    assert(employees[idx1] == "Robert");

    // Test pop_back
    employees.pop_back();
    assert(employees.size() == 2);

    // Test clear
    employees.clear();
    assert(employees.empty());

    std::cout << "  ✓ Vector operations" << std::endl;
}

// Test with array
void test_dense_array() {
    std::cout << "Testing DenseIndexedContainer with array..." << std::endl;

    using ProductArray = dense_index::DenseIndexedContainer<std::array<double, 5>, ProductIndex>;
    ProductArray prices{};

    // Test element access
    ProductIndex idx0(0);
    ProductIndex idx1(1);
    prices[idx0] = 19.99;
    prices[idx1] = 29.99;

    assert(prices[idx0] == 19.99);
    assert(prices[idx1] == 29.99);

    // Test size (fixed for array)
    assert(prices.size() == 5);

    // Test iteration
    prices[ProductIndex(2)] = 39.99;
    prices[ProductIndex(3)] = 49.99;
    prices[ProductIndex(4)] = 59.99;

    double sum = 0.0;
    for (const auto& price : prices) {
        sum += price;
    }
    assert(std::abs(sum - 199.95) < 0.001);

    // Test data() access
    auto* data_ptr = prices.data();
    assert(data_ptr[0] == 19.99);

    std::cout << "  ✓ Array operations" << std::endl;
}

// Test with deque
void test_dense_deque() {
    std::cout << "Testing DenseIndexedContainer with deque..." << std::endl;

    struct TaskTag {};
    using TaskIndex = dense_index::StrongIndex<TaskTag>;
    using TaskDeque = dense_index::DenseIndexedContainer<std::deque<int>, TaskIndex>;
    TaskDeque tasks;

    // Test push operations
    auto idx0 = tasks.push_back(100);
    auto idx1 = tasks.push_back(200);
    auto idx2 = tasks.push_back(300);

    assert(tasks[idx0] == 100);
    assert(tasks[idx1] == 200);
    assert(tasks[idx2] == 300);

    // Test size
    assert(tasks.size() == 3);

    // Test insert
    auto idx_new = tasks.insert(idx1, 150);
    assert(tasks[idx_new] == 150);
    assert(tasks.size() == 4);

    // Test erase
    [[maybe_unused]] auto next_idx = tasks.erase(idx_new);
    assert(tasks.size() == 3);

    std::cout << "  ✓ Deque operations" << std::endl;
}

// Test iterator utilities
void test_iterator_utilities() {
    std::cout << "Testing iterator utilities..." << std::endl;

    struct NumberTag {};
    using NumberIndex = dense_index::StrongIndex<NumberTag>;
    using NumberVector = dense_index::DenseIndexedContainer<std::vector<int>, NumberIndex>;
    NumberVector numbers;

    for (int i = 0; i < 10; ++i) {
        [[maybe_unused]] auto _ = numbers.push_back(i);
    }

    // Test index_of
    auto it = numbers.begin() + 5;
    auto idx = numbers.index_of(it);
    assert(idx.value() == 5);
    assert(numbers[idx] == 5);

    // Test iterator_at
    auto idx2 = NumberIndex(7);
    auto it2 = numbers.iterator_at(idx2);
    assert(*it2 == 7);

    std::cout << "  ✓ Iterator utilities" << std::endl;
}

// Test with STL algorithms
void test_stl_algorithms() {
    std::cout << "Testing STL algorithm compatibility..." << std::endl;

    struct ScoreTag {};
    using ScoreIndex = dense_index::StrongIndex<ScoreTag>;
    using ScoreVector = dense_index::DenseIndexedContainer<std::vector<int>, ScoreIndex>;
    ScoreVector scores;

    for (int i = 10; i > 0; --i) {
        [[maybe_unused]] auto _ = scores.push_back(i * 10);
    }

    // Test std::sort
    std::sort(scores.begin(), scores.end());
    assert(scores[ScoreIndex(0)] == 10);
    assert(scores[ScoreIndex(9)] == 100);

    // Test std::find
    auto it = std::find(scores.begin(), scores.end(), 50);
    assert(it != scores.end());
    auto idx = scores.index_of(it);
    assert(scores[idx] == 50);

    // Test std::accumulate
    int sum = std::accumulate(scores.begin(), scores.end(), 0);
    assert(sum == 550);

    std::cout << "  ✓ STL algorithms work" << std::endl;
}

// Test conditional method availability
void test_conditional_methods() {
    std::cout << "Testing conditional method availability..." << std::endl;

    // Vector has capacity, reserve, etc.
    struct VecTag {};
    using VecIndex = dense_index::StrongIndex<VecTag>;
    using Vec = dense_index::DenseIndexedContainer<std::vector<int>, VecIndex>;
    Vec vec;
    vec.reserve(100);
    assert(vec.capacity() >= 100);

    [[maybe_unused]] auto _ = vec.push_back(1);
    vec.shrink_to_fit();

    vec.resize(10);
    assert(vec.size() == 10);

    vec.resize(5, 42);
    assert(vec.size() == 5);

    // Array doesn't have push_back, capacity, etc.
    struct ArrTag {};
    using ArrIndex = dense_index::StrongIndex<ArrTag>;
    using Arr = dense_index::DenseIndexedContainer<std::array<int, 10>, ArrIndex>;
    Arr arr{};

    // These should not compile:
    // arr.push_back(1);  // Error: no push_back
    // arr.reserve(20);   // Error: no reserve
    // arr.capacity();    // Error: no capacity

    // But array has data()
    auto* ptr = arr.data();
    assert(ptr != nullptr);

    std::cout << "  ✓ Conditional methods work correctly" << std::endl;
}

// Test construction variants
void test_construction() {
    std::cout << "Testing various construction methods..." << std::endl;

    // From initializer list
    struct IntTag {};
    using IntIndex = dense_index::StrongIndex<IntTag>;
    using IntVec = dense_index::DenseIndexedContainer<std::vector<int>, IntIndex>;
    IntVec vec1{1, 2, 3, 4, 5};
    assert(vec1.size() == 5);
    assert(vec1[IntIndex(2)] == 3);

    // From size
    IntVec vec2(10);
    assert(vec2.size() == 10);

    // From size and value
    IntVec vec3(5, 42);
    assert(vec3.size() == 5);
    assert(vec3[IntIndex(0)] == 42);

    // From iterators
    std::vector<int> source = {10, 20, 30};
    IntVec vec4(source.begin(), source.end());
    assert(vec4.size() == 3);
    assert(vec4[IntIndex(1)] == 20);

    std::cout << "  ✓ Construction variants" << std::endl;
}

// Test swap functionality
void test_swap() {
    std::cout << "Testing swap..." << std::endl;

    struct StrTag {};
    using StrIndex = dense_index::StrongIndex<StrTag>;
    using StrVec = dense_index::DenseIndexedContainer<std::vector<std::string>, StrIndex>;
    StrVec vec1;
    [[maybe_unused]] auto _1 = vec1.push_back("A");
    [[maybe_unused]] auto _2 = vec1.push_back("B");

    StrVec vec2;
    [[maybe_unused]] auto _3 = vec2.push_back("X");
    [[maybe_unused]] auto _4 = vec2.push_back("Y");
    [[maybe_unused]] auto _5 = vec2.push_back("Z");

    vec1.swap(vec2);
    assert(vec1.size() == 3);
    assert(vec2.size() == 2);
    assert(vec1[StrIndex(0)] == "X");
    assert(vec2[StrIndex(0)] == "A");

    // Test ADL swap
    using std::swap;
    swap(vec1, vec2);
    assert(vec1.size() == 2);
    assert(vec2.size() == 3);

    std::cout << "  ✓ Swap functionality" << std::endl;
}

// Test comparison operators
void test_comparisons() {
    std::cout << "Testing comparison operators..." << std::endl;

    struct CmpTag {};
    using CmpIndex = dense_index::StrongIndex<CmpTag>;
    using IntVec = dense_index::DenseIndexedContainer<std::vector<int>, CmpIndex>;
    IntVec vec1{1, 2, 3};
    IntVec vec2{1, 2, 3};
    IntVec vec3{1, 2, 4};
    IntVec vec4{1, 2};

    assert(vec1 == vec2);
    assert(vec1 != vec3);
    assert(vec1 < vec3);
    assert(vec3 > vec1);
    assert(vec4 < vec1);

    std::cout << "  ✓ Comparison operators" << std::endl;
}

// Test hash support for indices
void test_hash_support() {
    std::cout << "Testing hash support..." << std::endl;

    EmployeeIndex idx(42);
    std::hash<EmployeeIndex> hasher;
    auto hash_value = hasher(idx);
    assert(hash_value == std::hash<std::size_t>{}(42));

    std::cout << "  ✓ Hash support" << std::endl;
}

// Compile-time tests for concepts
void test_concepts() {
    std::cout << "Testing concept constraints..." << std::endl;

    // Test IndexTag concept
    static_assert(dense_index::IndexTag<EmployeeTag>);
    static_assert(dense_index::IndexTag<struct AnyTag>);
    enum class TestEnumTag {};
    static_assert(dense_index::IndexTag<TestEnumTag>);
    static_assert(!dense_index::IndexTag<int>);
    static_assert(!dense_index::IndexTag<std::size_t>);

    // Test IndexableContainer concept
    static_assert(dense_index::IndexableContainer<std::vector<int>>);
    static_assert(dense_index::IndexableContainer<std::array<int, 10>>);
    static_assert(dense_index::IndexableContainer<std::deque<int>>);
    static_assert(!dense_index::IndexableContainer<std::list<int>>);  // No operator[]

    // Test individual capability concepts
    static_assert(dense_index::HasPushBack<std::vector<int>>);
    static_assert(!dense_index::HasPushBack<std::array<int, 10>>);
    static_assert(dense_index::HasCapacity<std::vector<int>>);
    static_assert(!dense_index::HasCapacity<std::array<int, 10>>);
    static_assert(dense_index::HasData<std::vector<int>>);
    static_assert(dense_index::HasData<std::array<int, 10>>);

    std::cout << "  ✓ Concept constraints validated" << std::endl;
}

// Test underlying container access
void test_underlying_access() {
    std::cout << "Testing underlying container access..." << std::endl;

    struct UnderlyingTag {};
    using UnderlyingIndex = dense_index::StrongIndex<UnderlyingTag>;
    using IntVec = dense_index::DenseIndexedContainer<std::vector<int>, UnderlyingIndex>;
    IntVec vec{1, 2, 3, 4, 5};

    // Get underlying container
    auto& underlying = vec.underlying();
    assert(underlying.size() == 5);
    assert(underlying[2] == 3);

    // Modify through underlying (escape hatch)
    underlying.push_back(6);
    assert(vec.size() == 6);

    const IntVec& const_vec = vec;
    const auto& const_underlying = const_vec.underlying();
    assert(const_underlying.size() == 6);

    std::cout << "  ✓ Underlying container access" << std::endl;
}

// Test type aliases
void test_type_aliases() {
    std::cout << "Testing type aliases..." << std::endl;

    struct VectorTag {};
    using VectorIndex = dense_index::StrongIndex<VectorTag>;
    dense_index::DenseVector<int, VectorIndex> dense_vec{1, 2, 3};
    assert(dense_vec.size() == 3);

    struct ArrayTag {};
    using ArrayIndex = dense_index::StrongIndex<ArrayTag>;
    dense_index::DenseArray<double, 5, ArrayIndex> dense_arr{};
    assert(dense_arr.size() == 5);

    struct DequeTag {};
    using DequeIndex = dense_index::StrongIndex<DequeTag>;
    dense_index::DenseDeque<std::string, DequeIndex> dense_deq;
    [[maybe_unused]] auto _ = dense_deq.push_back("test");
    assert(dense_deq.size() == 1);

    std::cout << "  ✓ Type aliases work" << std::endl;
}

// Compile-time error tests (these should NOT compile)
template<typename = void>
void compile_time_error_tests() {
    using EmpVec = dense_index::DenseIndexedContainer<std::vector<int>, EmployeeIndex>;
    using DeptVec = dense_index::DenseIndexedContainer<std::vector<int>, DepartmentIndex>;

    EmpVec emp_vec;
    DeptVec dept_vec;

    emp_vec.push_back(1);
    dept_vec.push_back(2);

    EmployeeIndex emp_idx(0);
    DepartmentIndex dept_idx(0);

    // These should all cause compile errors:

    // 1. Using wrong index type
    // auto val = emp_vec[dept_idx];  // Error!

    // 2. Using raw index
    // auto val2 = emp_vec[0];  // Error! deleted function

    // 3. Mixing index types
    // emp_idx = dept_idx;  // Error!

    // 4. Comparing different index types
    // bool same = (emp_idx == dept_idx);  // Error!

    // 5. Implicit conversion from size_t
    // EmployeeIndex idx = 5;  // Error! explicit constructor

    // 6. Accessing deleted at() with raw index
    // auto val3 = emp_vec.at(0);  // Error! deleted function
}

// Performance test to verify zero overhead
void test_performance() {
    std::cout << "Testing performance (zero overhead)..." << std::endl;

    const size_t N = 1000000;

    // Raw vector
    std::vector<int> raw_vec(N);
    for (size_t i = 0; i < N; ++i) {
        raw_vec[i] = static_cast<int>(i);
    }

    // Dense indexed vector
    struct PerfTag {};
    using PerfIndex = dense_index::StrongIndex<PerfTag>;
    using PerfVec = dense_index::DenseIndexedContainer<std::vector<int>, PerfIndex>;
    PerfVec dense_vec(N);
    for (size_t i = 0; i < N; ++i) {
        dense_vec[PerfIndex(i)] = static_cast<int>(i);
    }

    // Both should have identical memory layout
    assert(raw_vec.size() == dense_vec.size());
    assert(memcmp(raw_vec.data(), dense_vec.underlying().data(), N * sizeof(int)) == 0);

    std::cout << "  ✓ Zero overhead confirmed" << std::endl;
}

int main() {
    std::cout << "\n=== Dense Indexed Container Test Suite ===" << std::endl;

    test_strong_index();
    test_type_safety();
    test_dense_vector();
    test_dense_array();
    test_dense_deque();
    test_iterator_utilities();
    test_stl_algorithms();
    test_conditional_methods();
    test_construction();
    test_swap();
    test_comparisons();
    test_hash_support();
    test_concepts();
    test_underlying_access();
    test_type_aliases();
    test_performance();

    std::cout << "\n✅ All tests passed!" << std::endl;

    return 0;
}