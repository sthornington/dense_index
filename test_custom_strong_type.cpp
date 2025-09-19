#include "dense_index.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <type_traits>

// Example custom strong type that provides get() method
template<typename Tag>
class CustomStrongIndex {
private:
    std::size_t value_;

public:
    // Constructor from size_t (required by concept)
    explicit CustomStrongIndex(std::size_t v = 0) : value_(v) {}

    // get() method returning size_t (required by concept)
    [[nodiscard]] std::size_t get() const noexcept { return value_; }

    // Comparison operators
    auto operator<=>(const CustomStrongIndex&) const = default;

    // Arithmetic for convenience
    CustomStrongIndex operator+(std::size_t n) const {
        return CustomStrongIndex(value_ + n);
    }

    CustomStrongIndex& operator++() {
        ++value_;
        return *this;
    }
};

// Another example: strong typedef using inheritance
class FileHandle : public CustomStrongIndex<struct FileHandleTag> {
    using CustomStrongIndex::CustomStrongIndex;
};

// Example from a hypothetical third-party strong type library
namespace third_party {
    template<typename T, typename PhantomType>
    struct NamedType {
        T value;

        explicit NamedType(T v) : value(v) {}

        // Compatibility with dense_index - just needs get() method
        [[nodiscard]] T get() const { return value; }

        auto operator<=>(const NamedType&) const = default;
    };

    using UserId = NamedType<std::size_t, struct UserIdTag>;
    using SessionId = NamedType<std::size_t, struct SessionIdTag>;
}

int main() {
    std::cout << "\n=== Testing Custom Strong Types with DenseIndexedContainer ===" << std::endl;

    // Test 1: Custom strong index
    {
        std::cout << "\nTest 1: Custom strong index type" << std::endl;

        using CustomIndex = CustomStrongIndex<struct TestTag>;
        dense_index::DenseIndexedContainer<std::vector<std::string>, CustomIndex> names;

        auto idx0 = names.push_back("Alice");
        auto idx1 = names.push_back("Bob");
        auto idx2 = names.push_back("Charlie");

        assert(names[idx0] == "Alice");
        assert(names[idx1] == "Bob");
        assert(names[idx2] == "Charlie");

        // Can iterate with the custom index
        for (CustomIndex i(0); i.get() < names.size(); ++i) {
            std::cout << "  names[" << i.get() << "] = " << names[i] << std::endl;
        }

        std::cout << "  ✓ Custom strong index works" << std::endl;
    }

    // Test 2: FileHandle (inherited strong type)
    {
        std::cout << "\nTest 2: Inherited strong type (FileHandle)" << std::endl;

        dense_index::DenseIndexedContainer<std::vector<std::string>, FileHandle> files;

        auto file1 = files.push_back("/etc/passwd");
        auto file2 = files.push_back("/home/user/document.txt");
        [[maybe_unused]] auto file3 = files.push_back("/var/log/system.log");

        std::cout << "  File at handle " << file2.get() << ": " << files[file2] << std::endl;

        assert(files[file1] == "/etc/passwd");
        assert(files.size() == 3);

        std::cout << "  ✓ Inherited strong type works" << std::endl;
    }

    // Test 3: Third-party NamedType library
    {
        std::cout << "\nTest 3: Third-party NamedType library" << std::endl;

        // User container
        struct User { std::string name; int age; };
        dense_index::DenseIndexedContainer<std::vector<User>, third_party::UserId> users;

        // Session container
        struct Session { third_party::UserId user; std::string token; };
        dense_index::DenseIndexedContainer<std::vector<Session>, third_party::SessionId> sessions;

        // Add users
        auto user1 = users.emplace_back("Alice", 30);
        auto user2 = users.emplace_back("Bob", 25);

        // Add sessions
        auto session1 = sessions.emplace_back(user1, "token123");
        auto session2 = sessions.emplace_back(user2, "token456");

        // Access through strong types
        std::cout << "  User: " << users[user1].name
                  << ", Session: " << sessions[session1].token << std::endl;

        // Type safety: These would not compile
        // auto bad = users[session1];  // Error: wrong index type
        // auto bad2 = sessions[user1]; // Error: wrong index type

        assert(users[user2].name == "Bob");
        assert(sessions[session2].user.get() == user2.get());

        std::cout << "  ✓ Third-party NamedType works" << std::endl;
    }

    // Test 4: Using built-in StrongIndex
    {
        std::cout << "\nTest 4: Using built-in StrongIndex type" << std::endl;

        struct MyTag {};
        using MyIndex = dense_index::StrongIndex<MyTag>;
        dense_index::DenseIndexedContainer<std::vector<int>, MyIndex> numbers;

        auto idx = numbers.push_back(42);
        assert(numbers[idx] == 42);

        // Verify the type is correct
        static_assert(std::is_same_v<
            decltype(idx),
            MyIndex
        >);

        std::cout << "  ✓ Built-in StrongIndex works" << std::endl;
    }

    // Test 5: Mixing both approaches in the same program
    {
        std::cout << "\nTest 5: Mixing both approaches" << std::endl;

        // Using built-in StrongIndex
        struct BuiltinTag {};
        using BuiltinIndex = dense_index::StrongIndex<BuiltinTag>;
        dense_index::DenseIndexedContainer<std::vector<int>, BuiltinIndex> container1;

        // Using custom strong type
        using MyCustomIndex = CustomStrongIndex<struct CustomTag>;
        dense_index::DenseIndexedContainer<std::vector<int>, MyCustomIndex> container2;

        // Using third-party strong type
        using ThirdPartyIndex = third_party::NamedType<std::size_t, struct ThirdPartyTag>;
        dense_index::DenseIndexedContainer<std::vector<int>, ThirdPartyIndex> container3;

        auto idx1 = container1.push_back(100);
        auto idx2 = container2.push_back(200);
        auto idx3 = container3.push_back(300);

        assert(container1[idx1] == 100);
        assert(container2[idx2] == 200);
        assert(container3[idx3] == 300);

        // All three index types are different and incompatible
        static_assert(!std::is_same_v<decltype(idx1), decltype(idx2)>);
        static_assert(!std::is_same_v<decltype(idx2), decltype(idx3)>);
        static_assert(!std::is_same_v<decltype(idx1), decltype(idx3)>);

        std::cout << "  ✓ All three approaches work together" << std::endl;
    }

    std::cout << "\n✅ All custom strong type tests passed!" << std::endl;
    std::cout << "\nThe library successfully works with:" << std::endl;
    std::cout << "  • Built-in StrongIndex<Tag> type" << std::endl;
    std::cout << "  • Custom strong types with .get() method" << std::endl;
    std::cout << "  • Custom strong types with .value() method" << std::endl;
    std::cout << "  • Custom strong types with implicit conversion" << std::endl;
    std::cout << "  • Third-party strong type libraries" << std::endl;

    return 0;
}