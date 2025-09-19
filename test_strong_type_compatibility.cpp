#include "dense_index.hpp"
#include <iostream>
#include <vector>
#include <cassert>

// Style 1: BOOST_STRONG_TYPEDEF style (implicit conversion)
struct BoostStyleStrongType {
    std::size_t value_;

    explicit BoostStyleStrongType(std::size_t v = 0) : value_(v) {}

    // BOOST style: implicit conversion operator
    operator std::size_t() const { return value_; }

    auto operator<=>(const BoostStyleStrongType&) const = default;
};

// Style 2: NamedType style (uses .get())
template<typename Tag>
struct NamedTypeStyle {
    std::size_t value_;

    explicit NamedTypeStyle(std::size_t v = 0) : value_(v) {}

    // NamedType style: .get() method
    [[nodiscard]] std::size_t get() const { return value_; }

    auto operator<=>(const NamedTypeStyle&) const = default;
};

// Style 3: std::optional style (uses .value())
template<typename Tag>
struct OptionalStyle {
    std::size_t data_;

    explicit OptionalStyle(std::size_t v = 0) : data_(v) {}

    // std::optional style: .value() method
    [[nodiscard]] std::size_t value() const { return data_; }

    auto operator<=>(const OptionalStyle&) const = default;
};

// Style 4: Mixed style (has multiple accessors)
struct FlexibleStrongType {
    std::size_t val_;

    explicit FlexibleStrongType(std::size_t v = 0) : val_(v) {}

    // Provides all three access patterns!
    [[nodiscard]] std::size_t get() const { return val_; }
    [[nodiscard]] std::size_t value() const { return val_; }
    operator std::size_t() const { return val_; }

    auto operator<=>(const FlexibleStrongType&) const = default;
};

int main() {
    std::cout << "\n=== Testing Different Strong Type Styles ===" << std::endl;

    // Test 1: BOOST_STRONG_TYPEDEF style
    {
        std::cout << "\nTest 1: BOOST style (implicit conversion)" << std::endl;

        dense_index::DenseIndexedContainer<std::vector<std::string>, BoostStyleStrongType> names;

        auto idx0 = names.push_back("Alice");
        auto idx1 = names.push_back("Bob");

        assert(names[idx0] == "Alice");
        assert(names[idx1] == "Bob");

        // Can use the index directly where size_t is expected due to implicit conversion
        std::size_t raw = idx0;  // Works with BOOST style
        std::cout << "  Index 0 as size_t: " << raw << std::endl;

        std::cout << "  ✓ BOOST style works" << std::endl;
    }

    // Test 2: NamedType style
    {
        std::cout << "\nTest 2: NamedType style (.get() method)" << std::endl;

        using NamedIndex = NamedTypeStyle<struct NamedTag>;
        dense_index::DenseIndexedContainer<std::vector<int>, NamedIndex> values;

        auto idx = values.push_back(42);
        assert(values[idx] == 42);

        // Access via .get()
        std::cout << "  Index value via .get(): " << idx.get() << std::endl;

        std::cout << "  ✓ NamedType style works" << std::endl;
    }

    // Test 3: std::optional style
    {
        std::cout << "\nTest 3: std::optional style (.value() method)" << std::endl;

        using OptIndex = OptionalStyle<struct OptTag>;
        dense_index::DenseIndexedContainer<std::vector<double>, OptIndex> prices;

        auto idx = prices.push_back(19.99);
        assert(prices[idx] == 19.99);

        // Access via .value()
        std::cout << "  Index value via .value(): " << idx.value() << std::endl;

        std::cout << "  ✓ std::optional style works" << std::endl;
    }

    // Test 4: Flexible strong type with all accessors
    {
        std::cout << "\nTest 4: Flexible type (all access methods)" << std::endl;

        dense_index::DenseIndexedContainer<std::vector<char>, FlexibleStrongType> chars;

        auto idx = chars.push_back('X');
        assert(chars[idx] == 'X');

        // All three access patterns work
        std::cout << "  Via .get(): " << idx.get() << std::endl;
        std::cout << "  Via .value(): " << idx.value() << std::endl;
        std::cout << "  Via implicit: " << static_cast<std::size_t>(idx) << std::endl;

        std::cout << "  ✓ Flexible type works" << std::endl;
    }

    // Test 5: All in one program
    {
        std::cout << "\nTest 5: Using all styles together" << std::endl;

        dense_index::DenseIndexedContainer<std::vector<int>, BoostStyleStrongType> boost_container;
        dense_index::DenseIndexedContainer<std::vector<int>, NamedTypeStyle<struct Tag1>> named_container;
        dense_index::DenseIndexedContainer<std::vector<int>, OptionalStyle<struct Tag2>> optional_container;
        dense_index::DenseIndexedContainer<std::vector<int>, FlexibleStrongType> flexible_container;

        auto idx1 = boost_container.push_back(1);
        auto idx2 = named_container.push_back(2);
        auto idx3 = optional_container.push_back(3);
        auto idx4 = flexible_container.push_back(4);

        assert(boost_container[idx1] == 1);
        assert(named_container[idx2] == 2);
        assert(optional_container[idx3] == 3);
        assert(flexible_container[idx4] == 4);

        std::cout << "  ✓ All styles work together" << std::endl;
    }

    // Verify concept detection
    {
        std::cout << "\nVerifying concept detection:" << std::endl;

        static_assert(dense_index::StrongIndexType<BoostStyleStrongType>);
        static_assert(dense_index::StrongIndexType<NamedTypeStyle<struct T>>);
        static_assert(dense_index::StrongIndexType<OptionalStyle<struct T>>);
        static_assert(dense_index::StrongIndexType<FlexibleStrongType>);
        static_assert(dense_index::StrongIndexType<dense_index::StrongIndex<struct T>>);

        // Regular types don't satisfy the concept
        static_assert(!dense_index::StrongIndexType<std::size_t>);
        static_assert(!dense_index::StrongIndexType<int>);

        std::cout << "  ✓ Concept correctly identifies strong types" << std::endl;
    }

    std::cout << "\n✅ All compatibility tests passed!" << std::endl;
    std::cout << "\nThe library supports strong types with:" << std::endl;
    std::cout << "  • .get() method (NamedType style)" << std::endl;
    std::cout << "  • .value() method (std::optional style)" << std::endl;
    std::cout << "  • Implicit conversion (BOOST_STRONG_TYPEDEF style)" << std::endl;
    std::cout << "  • Any combination of the above" << std::endl;

    return 0;
}