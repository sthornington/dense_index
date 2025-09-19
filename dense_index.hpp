#pragma once

#include <concepts>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <compare>
#include <ranges>
#include <vector>
#include <array>
#include <deque>

namespace dense_index {

// Concept for any strong index type with various access patterns
template<typename T>
concept StrongIndexType =
    // Must not be a raw integral type
    !std::is_integral_v<T> &&
    // Should be constructible from size_t (for return values)
    requires(T t, const T ct, std::size_t n) {
        { T{n} };
    } && (
        requires(const T ct) {
            // Option 1: has .get() method (NamedType style)
            { ct.get() } -> std::convertible_to<std::size_t>;
        } ||
        requires(const T ct) {
            // Option 2: has .value() method (std::optional style)
            { ct.value() } -> std::convertible_to<std::size_t>;
        } ||
        requires(const T ct) {
            // Option 3: implicitly convertible (BOOST_STRONG_TYPEDEF style)
            // But not a raw integral type (checked above)
            { static_cast<std::size_t>(ct) } -> std::convertible_to<std::size_t>;
        }
    );

// Helper to get the value from any strong index type
template<typename T>
constexpr std::size_t get_index_value(const T& idx) {
    if constexpr (requires { idx.get(); }) {
        return idx.get();
    } else if constexpr (requires { idx.value(); }) {
        return idx.value();
    } else {
        return static_cast<std::size_t>(idx);
    }
}

// Concept for tag types used to differentiate index domains
template<typename T>
concept IndexTag = std::is_class_v<T> || std::is_enum_v<T>;

// Strong index type with C++23 features
template<IndexTag Tag>
class StrongIndex {
public:
    using tag_type = Tag;
    using underlying_type = std::size_t;

private:
    underlying_type value_{};

public:
    // Constructors
    constexpr StrongIndex() noexcept = default;
    constexpr explicit StrongIndex(underlying_type value) noexcept : value_(value) {}

    // Conversion operators
    [[nodiscard]] constexpr explicit operator underlying_type() const noexcept {
        return value_;
    }

    [[nodiscard]] constexpr underlying_type value() const noexcept {
        return value_;
    }

    // Provide get() method for compatibility with generic strong type concept
    [[nodiscard]] constexpr underlying_type get() const noexcept {
        return value_;
    }

    // Spaceship operator for comparisons (C++20)
    [[nodiscard]] constexpr auto operator<=>(const StrongIndex&) const noexcept = default;
    [[nodiscard]] constexpr bool operator==(const StrongIndex&) const noexcept = default;

    // Increment/decrement operators
    constexpr StrongIndex& operator++() noexcept {
        ++value_;
        return *this;
    }

    constexpr StrongIndex operator++(int) noexcept {
        StrongIndex tmp(*this);
        ++value_;
        return tmp;
    }

    constexpr StrongIndex& operator--() noexcept {
        --value_;
        return *this;
    }

    constexpr StrongIndex operator--(int) noexcept {
        StrongIndex tmp(*this);
        --value_;
        return tmp;
    }

    // Arithmetic operations
    [[nodiscard]] constexpr StrongIndex operator+(underlying_type n) const noexcept {
        return StrongIndex(value_ + n);
    }

    [[nodiscard]] constexpr StrongIndex operator-(underlying_type n) const noexcept {
        return StrongIndex(value_ - n);
    }

    constexpr StrongIndex& operator+=(underlying_type n) noexcept {
        value_ += n;
        return *this;
    }

    constexpr StrongIndex& operator-=(underlying_type n) noexcept {
        value_ -= n;
        return *this;
    }

    [[nodiscard]] constexpr std::ptrdiff_t operator-(StrongIndex other) const noexcept {
        return static_cast<std::ptrdiff_t>(value_) - static_cast<std::ptrdiff_t>(other.value_);
    }
};

} // namespace dense_index

// Helper to make index types hashable
template<dense_index::IndexTag Tag>
struct std::hash<dense_index::StrongIndex<Tag>> {
    [[nodiscard]] std::size_t operator()(const dense_index::StrongIndex<Tag>& idx) const noexcept {
        return std::hash<typename dense_index::StrongIndex<Tag>::underlying_type>{}(idx.value());
    }
};

namespace dense_index {

// Concepts for container requirements
template<typename C>
concept HasIndexOperator = requires(C& c, const C& cc, std::size_t i) {
    { c[i] } -> std::convertible_to<typename C::reference>;
    { cc[i] } -> std::convertible_to<typename C::const_reference>;
};

template<typename C>
concept HasAt = requires(C& c, const C& cc, std::size_t i) {
    { c.at(i) } -> std::convertible_to<typename C::reference>;
    { cc.at(i) } -> std::convertible_to<typename C::const_reference>;
};

template<typename C>
concept HasSize = requires(const C& c) {
    { c.size() } -> std::convertible_to<std::size_t>;
};

template<typename C>
concept HasEmpty = requires(const C& c) {
    { c.empty() } -> std::convertible_to<bool>;
};

template<typename C>
concept HasCapacity = requires(const C& c) {
    { c.capacity() } -> std::convertible_to<std::size_t>;
};

template<typename C>
concept HasReserve = requires(C& c, std::size_t n) {
    { c.reserve(n) } -> std::same_as<void>;
};

template<typename C>
concept HasClear = requires(C& c) {
    { c.clear() } -> std::same_as<void>;
};

template<typename C>
concept HasPushBack = requires(C& c, typename C::value_type v) {
    { c.push_back(v) } -> std::same_as<void>;
    { c.push_back(std::move(v)) } -> std::same_as<void>;
};

template<typename C>
concept HasPopBack = requires(C& c) {
    { c.pop_back() } -> std::same_as<void>;
};

template<typename C>
concept HasEmplaceBack = requires(C& c) {
    c.emplace_back();
};

template<typename C>
concept HasFront = requires(C& c, const C& cc) {
    { c.front() } -> std::convertible_to<typename C::reference>;
    { cc.front() } -> std::convertible_to<typename C::const_reference>;
};

template<typename C>
concept HasBack = requires(C& c, const C& cc) {
    { c.back() } -> std::convertible_to<typename C::reference>;
    { cc.back() } -> std::convertible_to<typename C::const_reference>;
};

template<typename C>
concept HasResize = requires(C& c, std::size_t n, typename C::value_type v) {
    { c.resize(n) } -> std::same_as<void>;
    { c.resize(n, v) } -> std::same_as<void>;
};

template<typename C>
concept HasInsert = requires(C& c, typename C::iterator it, typename C::value_type v) {
    { c.insert(it, v) } -> std::convertible_to<typename C::iterator>;
    { c.insert(it, std::move(v)) } -> std::convertible_to<typename C::iterator>;
};

template<typename C>
concept HasErase = requires(C& c, typename C::iterator it) {
    { c.erase(it) } -> std::convertible_to<typename C::iterator>;
    { c.erase(it, it) } -> std::convertible_to<typename C::iterator>;
};

template<typename C>
concept HasData = requires(C& c, const C& cc) {
    { c.data() } -> std::convertible_to<typename C::value_type*>;
    { cc.data() } -> std::convertible_to<const typename C::value_type*>;
};

template<typename C>
concept HasShrinkToFit = requires(C& c) {
    { c.shrink_to_fit() } -> std::same_as<void>;
};

// Main container concept
template<typename C>
concept IndexableContainer = requires(C& c, const C& cc) {
    typename C::value_type;
    typename C::reference;
    typename C::const_reference;
    typename C::size_type;
    typename C::iterator;
    typename C::const_iterator;

    { c.begin() } -> std::convertible_to<typename C::iterator>;
    { c.end() } -> std::convertible_to<typename C::iterator>;
    { cc.begin() } -> std::convertible_to<typename C::const_iterator>;
    { cc.end() } -> std::convertible_to<typename C::const_iterator>;
} && HasIndexOperator<C> && HasSize<C>;

// Dense indexed container - requires a strong index type
template<IndexableContainer Container, StrongIndexType IndexType>
class DenseIndexedContainer {
public:
    using container_type = Container;
    using index_type = IndexType;
    using value_type = typename Container::value_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using size_type = typename Container::size_type;
    using difference_type = typename Container::difference_type;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

private:
    Container container_;

public:
    // Constructors
    constexpr DenseIndexedContainer() = default;

    constexpr explicit DenseIndexedContainer(const Container& c) : container_(c) {}
    constexpr explicit DenseIndexedContainer(Container&& c) noexcept(std::is_nothrow_move_constructible_v<Container>)
        : container_(std::move(c)) {}

    // Constructor from iterators if container supports it
    template<typename InputIt>
        requires std::constructible_from<Container, InputIt, InputIt>
    constexpr DenseIndexedContainer(InputIt first, InputIt last) : container_(first, last) {}

    // Constructor with size if container supports it
    explicit DenseIndexedContainer(size_type count)
        requires std::constructible_from<Container, size_type>
        : container_(count) {}

    // Constructor with size and value if container supports it
    DenseIndexedContainer(size_type count, const value_type& value)
        requires std::constructible_from<Container, size_type, value_type>
        : container_(count, value) {}

    // Initializer list constructor if container supports it
    DenseIndexedContainer(std::initializer_list<value_type> init)
        requires std::constructible_from<Container, std::initializer_list<value_type>>
        : container_(init) {}

    // Element access
    [[nodiscard]] constexpr reference operator[](index_type idx) requires HasIndexOperator<Container> {
        return container_[get_index_value(idx)];
    }

    [[nodiscard]] constexpr const_reference operator[](index_type idx) const requires HasIndexOperator<Container> {
        return container_[get_index_value(idx)];
    }

    // Delete raw index access to enforce type safety
    reference operator[](size_type) = delete;
    const_reference operator[](size_type) const = delete;

    // at() with bounds checking
    [[nodiscard]] constexpr reference at(index_type idx) requires HasAt<Container> {
        return container_.at(get_index_value(idx));
    }

    [[nodiscard]] constexpr const_reference at(index_type idx) const requires HasAt<Container> {
        return container_.at(get_index_value(idx));
    }

    // Delete raw index at() access
    template<typename C = Container>
        requires HasAt<C>
    reference at(size_type) = delete;

    template<typename C = Container>
        requires HasAt<C>
    const_reference at(size_type) const = delete;

    // Front and back access
    [[nodiscard]] constexpr reference front() requires HasFront<Container> {
        return container_.front();
    }

    [[nodiscard]] constexpr const_reference front() const requires HasFront<Container> {
        return container_.front();
    }

    [[nodiscard]] constexpr reference back() requires HasBack<Container> {
        return container_.back();
    }

    [[nodiscard]] constexpr const_reference back() const requires HasBack<Container> {
        return container_.back();
    }

    // Data access
    [[nodiscard]] constexpr value_type* data() noexcept requires HasData<Container> {
        return container_.data();
    }

    [[nodiscard]] constexpr const value_type* data() const noexcept requires HasData<Container> {
        return container_.data();
    }

    // Iterators
    [[nodiscard]] constexpr iterator begin() noexcept { return container_.begin(); }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return container_.begin(); }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return container_.cbegin(); }

    [[nodiscard]] constexpr iterator end() noexcept { return container_.end(); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return container_.end(); }
    [[nodiscard]] constexpr const_iterator cend() const noexcept { return container_.cend(); }

    // Reverse iterators if container supports them
    [[nodiscard]] constexpr auto rbegin() noexcept
        requires requires(Container& c) { c.rbegin(); }
    { return container_.rbegin(); }

    [[nodiscard]] constexpr auto rbegin() const noexcept
        requires requires(const Container& c) { c.rbegin(); }
    { return container_.rbegin(); }

    [[nodiscard]] constexpr auto crbegin() const noexcept
        requires requires(const Container& c) { c.crbegin(); }
    { return container_.crbegin(); }

    [[nodiscard]] constexpr auto rend() noexcept
        requires requires(Container& c) { c.rend(); }
    { return container_.rend(); }

    [[nodiscard]] constexpr auto rend() const noexcept
        requires requires(const Container& c) { c.rend(); }
    { return container_.rend(); }

    [[nodiscard]] constexpr auto crend() const noexcept
        requires requires(const Container& c) { c.crend(); }
    { return container_.crend(); }

    // Capacity
    [[nodiscard]] constexpr bool empty() const noexcept requires HasEmpty<Container> {
        return container_.empty();
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return container_.size();
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
        requires requires(const Container& c) { c.max_size(); }
    {
        return container_.max_size();
    }

    [[nodiscard]] constexpr size_type capacity() const noexcept requires HasCapacity<Container> {
        return container_.capacity();
    }

    constexpr void reserve(size_type new_cap) requires HasReserve<Container> {
        container_.reserve(new_cap);
    }

    constexpr void shrink_to_fit() requires HasShrinkToFit<Container> {
        container_.shrink_to_fit();
    }

    // Modifiers
    constexpr void clear() noexcept requires HasClear<Container> {
        container_.clear();
    }

    // Insert with index return
    constexpr index_type insert(index_type pos, const value_type& value) requires HasInsert<Container> {
        auto it = container_.begin() + static_cast<difference_type>(get_index_value(pos));
        auto result_it = container_.insert(it, value);
        return index_type(std::distance(container_.begin(), result_it));
    }

    constexpr index_type insert(index_type pos, value_type&& value) requires HasInsert<Container> {
        auto it = container_.begin() + static_cast<difference_type>(get_index_value(pos));
        auto result_it = container_.insert(it, std::move(value));
        return index_type(std::distance(container_.begin(), result_it));
    }

    // Insert range
    template<typename InputIt>
        requires HasInsert<Container> && requires(Container& c, iterator it, InputIt first, InputIt last) {
            { c.insert(it, first, last) };
        }
    constexpr index_type insert(index_type pos, InputIt first, InputIt last) {
        auto it = container_.begin() + static_cast<difference_type>(get_index_value(pos));
        auto result_it = container_.insert(it, first, last);
        return index_type(std::distance(container_.begin(), result_it));
    }

    // Emplace
    template<typename... Args>
        requires requires(Container& c, iterator it, Args&&... args) {
            { c.emplace(it, std::forward<Args>(args)...) };
        }
    constexpr index_type emplace(index_type pos, Args&&... args) {
        auto it = container_.begin() + static_cast<difference_type>(get_index_value(pos));
        auto result_it = container_.emplace(it, std::forward<Args>(args)...);
        return index_type(std::distance(container_.begin(), result_it));
    }

    // Erase operations
    constexpr index_type erase(index_type pos) requires HasErase<Container> {
        auto it = container_.begin() + static_cast<difference_type>(get_index_value(pos));
        auto result_it = container_.erase(it);
        return index_type(std::distance(container_.begin(), result_it));
    }

    constexpr index_type erase(index_type first, index_type last) requires HasErase<Container> {
        auto first_it = container_.begin() + static_cast<difference_type>(get_index_value(first));
        auto last_it = container_.begin() + static_cast<difference_type>(get_index_value(last));
        auto result_it = container_.erase(first_it, last_it);
        return index_type(std::distance(container_.begin(), result_it));
    }

    // Push/pop operations with index return
    [[nodiscard]] constexpr index_type push_back(const value_type& value) requires HasPushBack<Container> {
        container_.push_back(value);
        return index_type(container_.size() - 1);
    }

    [[nodiscard]] constexpr index_type push_back(value_type&& value) requires HasPushBack<Container> {
        container_.push_back(std::move(value));
        return index_type(container_.size() - 1);
    }

    template<typename... Args>
    [[nodiscard]] constexpr index_type emplace_back(Args&&... args) requires HasEmplaceBack<Container> {
        container_.emplace_back(std::forward<Args>(args)...);
        return index_type(container_.size() - 1);
    }

    constexpr void pop_back() requires HasPopBack<Container> {
        container_.pop_back();
    }

    // Resize operations
    constexpr void resize(size_type count) requires HasResize<Container> {
        container_.resize(count);
    }

    constexpr void resize(size_type count, const value_type& value) requires HasResize<Container> {
        container_.resize(count, value);
    }

    // Swap
    constexpr void swap(DenseIndexedContainer& other)
        noexcept(std::is_nothrow_swappable_v<Container>)
        requires std::swappable<Container>
    {
        using std::swap;
        swap(container_, other.container_);
    }

    // Utility functions
    [[nodiscard]] constexpr index_type index_of(const_iterator it) const {
        auto dist = std::distance(container_.cbegin(), it);
        return index_type(static_cast<size_type>(dist));
    }

    [[nodiscard]] constexpr iterator iterator_at(index_type idx) {
        return container_.begin() + static_cast<difference_type>(get_index_value(idx));
    }

    [[nodiscard]] constexpr const_iterator iterator_at(index_type idx) const {
        return container_.cbegin() + static_cast<difference_type>(get_index_value(idx));
    }

    // Access to underlying container (escape hatch)
    [[nodiscard]] constexpr Container& underlying() noexcept { return container_; }
    [[nodiscard]] constexpr const Container& underlying() const noexcept { return container_; }

    // Comparison operators (if container supports them)
    [[nodiscard]] friend constexpr bool operator==(const DenseIndexedContainer& lhs, const DenseIndexedContainer& rhs)
        requires std::equality_comparable<Container>
    {
        return lhs.container_ == rhs.container_;
    }

    [[nodiscard]] friend constexpr auto operator<=>(const DenseIndexedContainer& lhs, const DenseIndexedContainer& rhs)
        requires std::three_way_comparable<Container>
    {
        return lhs.container_ <=> rhs.container_;
    }
};

// Swap specialization
template<IndexableContainer Container, StrongIndexType IndexType>
constexpr void swap(DenseIndexedContainer<Container, IndexType>& lhs, DenseIndexedContainer<Container, IndexType>& rhs)
    noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

} // namespace dense_index

// Range support
template<dense_index::IndexableContainer Container, dense_index::StrongIndexType IndexType>
inline constexpr bool std::ranges::enable_borrowed_range<dense_index::DenseIndexedContainer<Container, IndexType>> =
    std::ranges::enable_borrowed_range<Container>;

namespace dense_index {

// Type aliases - now require explicit strong index type
template<typename T, StrongIndexType IndexType>
using DenseVector = DenseIndexedContainer<std::vector<T>, IndexType>;

template<typename T, std::size_t N, StrongIndexType IndexType>
using DenseArray = DenseIndexedContainer<std::array<T, N>, IndexType>;

template<typename T, StrongIndexType IndexType>
using DenseDeque = DenseIndexedContainer<std::deque<T>, IndexType>;

} // namespace dense_index