// This file contains code that should NOT compile
// It demonstrates the type safety guarantees of the dense_index library
// Uncomment individual sections to verify compile-time errors

#include "dense_index.hpp"
#include <vector>
#include <string>

using namespace dense_index;

// Define test tags
struct EmployeeTag {};
struct DepartmentTag {};

using EmployeeIndex = StrongIndex<EmployeeTag>;
using DepartmentIndex = StrongIndex<DepartmentTag>;

using EmployeeVector = DenseIndexedContainer<std::vector<std::string>, EmployeeTag>;
using DepartmentVector = DenseIndexedContainer<std::vector<int>, DepartmentTag>;

void test_compile_time_errors() {
    EmployeeVector employees;
    DepartmentVector departments;

    auto emp_idx = employees.push_back("Alice");
    auto dept_idx = departments.push_back(100);

    // ========================================
    // ERROR 1: Using wrong index type
    // ========================================
    // Uncomment to see error: no match for 'operator[]' with DepartmentIndex
    // std::string name = employees[dept_idx];

    // ========================================
    // ERROR 2: Using raw index (size_t)
    // ========================================
    // Uncomment to see error: use of deleted function
    // std::string name2 = employees[0];
    // std::string name3 = employees[size_t(0)];

    // ========================================
    // ERROR 3: Using raw index with at()
    // ========================================
    // Uncomment to see error: use of deleted function
    // std::string name4 = employees.at(0);
    // std::string name5 = employees.at(size_t(0));

    // ========================================
    // ERROR 4: Mixing index types in assignment
    // ========================================
    // Uncomment to see error: no match for 'operator='
    // emp_idx = dept_idx;

    // ========================================
    // ERROR 5: Comparing different index types
    // ========================================
    // Uncomment to see error: no match for 'operator=='
    // bool same = (emp_idx == dept_idx);

    // ========================================
    // ERROR 6: Implicit conversion from size_t
    // ========================================
    // Uncomment to see error: conversion from 'int' to non-scalar type
    // EmployeeIndex idx1 = 5;
    // EmployeeIndex idx2 = size_t(5);

    // ========================================
    // ERROR 7: Implicit conversion to size_t
    // ========================================
    // Uncomment to see error: cannot convert StrongIndex to size_t
    // size_t raw_idx = emp_idx;

    // ========================================
    // ERROR 8: Using method that doesn't exist on underlying container
    // ========================================
    using ArrayContainer = DenseIndexedContainer<std::array<int, 10>, struct ArrayTag>;
    ArrayContainer arr{};

    // Uncomment to see error: no member named 'push_back' (requires constraint not satisfied)
    // arr.push_back(42);

    // Uncomment to see error: no member named 'capacity' (requires constraint not satisfied)
    // auto cap = arr.capacity();

    // Uncomment to see error: no member named 'reserve' (requires constraint not satisfied)
    // arr.reserve(20);

    // ========================================
    // ERROR 9: Invalid container type (no operator[])
    // ========================================
    // Uncomment to see error: constraints not satisfied for IndexableContainer
    // using ListContainer = DenseIndexedContainer<std::list<int>, struct ListTag>;

    // ========================================
    // ERROR 10: Invalid index tag type (not a class/enum)
    // ========================================
    // Uncomment to see error: constraints not satisfied for IndexTag
    // using BadIndex = StrongIndex<int>;
    // using BadContainer = DenseIndexedContainer<std::vector<int>, int>;
}

// ========================================
// Positive compile-time tests (these SHOULD compile)
// ========================================
void test_positive_cases() {
    // Different containers with same index type is OK
    DenseIndexedContainer<std::vector<int>, EmployeeTag> emp_ids;
    DenseIndexedContainer<std::vector<std::string>, EmployeeTag> emp_names;

    auto idx = emp_ids.push_back(1001);
    std::string name = emp_names[idx];  // OK - same index type

    // Explicit conversion is OK
    EmployeeIndex idx2(5);
    size_t raw = static_cast<size_t>(idx2);  // OK - explicit
    size_t raw2 = idx2.value();  // OK - explicit method

    // Construction from size_t is OK if explicit
    EmployeeIndex idx3{10};  // OK - explicit construction
    EmployeeIndex idx4(size_t(15));  // OK - explicit construction

    // Comparing same types is OK
    bool same = (idx2 == idx3);  // OK
    bool less = (idx2 < idx3);   // OK

    // Using conditional methods when available
    DenseIndexedContainer<std::vector<int>, struct VecTag> vec;
    vec.push_back(1);  // OK - vector has push_back
    vec.reserve(10);   // OK - vector has reserve
    auto cap = vec.capacity();  // OK - vector has capacity

    // Using enum class as tag
    enum class TaskPriority {};
    using TaskIndex = StrongIndex<TaskPriority>;
    DenseIndexedContainer<std::vector<int>, TaskPriority> tasks;
}

int main() {
    // This file is for compile-time testing only
    // Uncomment specific error cases to verify they produce appropriate errors
    return 0;
}