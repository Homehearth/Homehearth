#ifndef ENTT_ENTITY_STORAGE_HPP
#define ENTT_ENTITY_STORAGE_HPP


#include <cstddef>
#include <iterator>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include "../config/config.h"
#include "../core/algorithm.hpp"
#include "../core/compressed_pair.hpp"
#include "../core/memory.hpp"
#include "../core/type_traits.hpp"
#include "../signal/sigh.hpp"
#include "component.hpp"
#include "entity.hpp"
#include "fwd.hpp"
#include "sparse_set.hpp"


namespace entt {


/**
 * @brief Basic storage implementation.
 *
 * This class is a refinement of a sparse set that associates an object to an
 * entity. The main purpose of this class is to extend sparse sets to store
 * components in a registry. It guarantees fast access both to the elements and
 * to the entities.
 *
 * @note
 * Entities and objects have the same order.
 *
 * @note
 * Internal data structures arrange elements to maximize performance. There are
 * no guarantees that objects are returned in the insertion order when iterate
 * a storage. Do not make assumption on the order in any case.
 *
 * @warning
 * Empty types aren't explicitly instantiated. Therefore, many of the functions
 * normally available for non-empty types will not be available for empty ones.
 *
 * @sa sparse_set<Entity>
 *
 * @tparam Entity A valid entity type (see entt_traits for more details).
 * @tparam Type Type of objects assigned to the entities.
 * @tparam Allocator Type of allocator used to manage memory and elements.
 */
template<typename Entity, typename Type, typename Allocator, typename>
class basic_storage: public basic_sparse_set<Entity, typename std::allocator_traits<Allocator>::template rebind_alloc<Entity>> {
    static constexpr auto packed_page = ENTT_PACKED_PAGE;

    using allocator_traits = std::allocator_traits<Allocator>;

    using alloc = typename allocator_traits::template rebind_alloc<Type>;
    using alloc_traits = typename std::allocator_traits<alloc>;
    using alloc_const_pointer = typename alloc_traits::const_pointer;
    using alloc_pointer = typename alloc_traits::pointer;

    using alloc_ptr = typename allocator_traits::template rebind_alloc<alloc_pointer>;
    using alloc_ptr_traits = typename std::allocator_traits<alloc_ptr>;
    using alloc_ptr_const_pointer = typename allocator_traits::template rebind_traits<alloc_const_pointer>::const_pointer;
    using alloc_ptr_pointer = typename alloc_ptr_traits::pointer;

    using difference_type = typename entt_traits<Entity>::difference_type;
    using comp_traits = component_traits<Type>;

    using underlying_type = basic_sparse_set<Entity, typename allocator_traits::template rebind_alloc<Entity>>;

    static_assert(alloc_traits::propagate_on_container_move_assignment::value);
    static_assert(alloc_ptr_traits::propagate_on_container_move_assignment::value);

    template<typename Value>
    struct storage_iterator final {
        using difference_type = typename basic_storage::difference_type;
        using value_type = Value;
        using pointer = value_type *;
        using reference = value_type &;
        using iterator_category = std::random_access_iterator_tag;

        storage_iterator() ENTT_NOEXCEPT = default;

        storage_iterator(alloc_ptr_pointer const *ref, const typename basic_storage::difference_type idx) ENTT_NOEXCEPT
            : packed{ref},
              index{idx}
        {}

        storage_iterator & operator++() ENTT_NOEXCEPT {
            return --index, *this;
        }

        storage_iterator operator++(int) ENTT_NOEXCEPT {
            storage_iterator orig = *this;
            return ++(*this), orig;
        }

        storage_iterator & operator--() ENTT_NOEXCEPT {
            return ++index, *this;
        }

        storage_iterator operator--(int) ENTT_NOEXCEPT {
            storage_iterator orig = *this;
            return operator--(), orig;
        }

        storage_iterator & operator+=(const difference_type value) ENTT_NOEXCEPT {
            index -= value;
            return *this;
        }

        storage_iterator operator+(const difference_type value) const ENTT_NOEXCEPT {
            storage_iterator copy = *this;
            return (copy += value);
        }

        storage_iterator & operator-=(const difference_type value) ENTT_NOEXCEPT {
            return (*this += -value);
        }

        storage_iterator operator-(const difference_type value) const ENTT_NOEXCEPT {
            return (*this + -value);
        }

        difference_type operator-(const storage_iterator &other) const ENTT_NOEXCEPT {
            return other.index - index;
        }

        [[nodiscard]] reference operator[](const difference_type value) const ENTT_NOEXCEPT {
            const auto pos = size_type(index-value-1);
            return (*packed)[page(pos)][offset(pos)];
        }

        [[nodiscard]] bool operator==(const storage_iterator &other) const ENTT_NOEXCEPT {
            return other.index == index;
        }

        [[nodiscard]] bool operator!=(const storage_iterator &other) const ENTT_NOEXCEPT {
            return !(*this == other);
        }

        [[nodiscard]] bool operator<(const storage_iterator &other) const ENTT_NOEXCEPT {
            return index > other.index;
        }

        [[nodiscard]] bool operator>(const storage_iterator &other) const ENTT_NOEXCEPT {
            return index < other.index;
        }

        [[nodiscard]] bool operator<=(const storage_iterator &other) const ENTT_NOEXCEPT {
            return !(*this > other);
        }

        [[nodiscard]] bool operator>=(const storage_iterator &other) const ENTT_NOEXCEPT {
            return !(*this < other);
        }

        [[nodiscard]] pointer operator->() const ENTT_NOEXCEPT {
            const auto pos = size_type(index-1u);
            return std::addressof((*packed)[page(pos)][offset(pos)]);
        }

        [[nodiscard]] reference operator*() const ENTT_NOEXCEPT {
            return *operator->();
        }

    private:
        alloc_ptr_pointer const *packed;
        difference_type index;
    };

    [[nodiscard]] static auto page(const std::size_t pos) ENTT_NOEXCEPT {
        return pos / packed_page;
    }

    [[nodiscard]] static auto offset(const std::size_t pos) ENTT_NOEXCEPT {
        return pos & (packed_page - 1);
    }

    void release_memory() {
        if(packed) {
            // no-throw stable erase iteration
            base_type::clear();

            auto &allocator = bucket.first();
            auto &len = bucket.second();
            alloc_ptr allocator_ptr{allocator};

            for(size_type pos{}; pos < len; ++pos) {
                alloc_traits::deallocate(allocator, packed[pos], packed_page);
                alloc_ptr_traits::destroy(allocator_ptr, std::addressof(packed[pos]));
            }

            alloc_ptr_traits::deallocate(allocator_ptr, packed, len);
        }
    }

    void assure_at_least(const std::size_t pos) {
        if(const auto idx = page(pos); !(idx < bucket.second())) {
            auto &allocator = bucket.first();
            auto &len = bucket.second();
            alloc_ptr allocator_ptr{allocator};

            const size_type sz = idx + 1u;
            const auto mem = alloc_ptr_traits::allocate(allocator_ptr, sz);
            std::uninitialized_value_construct(mem + len, mem + sz);

            ENTT_TRY {
                for(auto next = len; next < sz; ++next) {
                    mem[next] = alloc_traits::allocate(allocator, packed_page);
                }
            } ENTT_CATCH {
                for(auto next = len; next < sz && mem[next]; ++next) {
                    alloc_traits::deallocate(allocator, mem[next], packed_page);
                }

                std::destroy(mem + len, mem + sz);
                alloc_ptr_traits::deallocate(allocator_ptr, mem, sz);
                ENTT_THROW;
            }

            if(packed) {
                std::uninitialized_copy(packed, packed + len, mem);
                std::destroy(packed, packed + len);
                alloc_ptr_traits::deallocate(allocator_ptr, packed, len);
            }

            packed = mem;
            len = sz;
        }
    }

    void release_unused_pages() {
        if(const auto length = base_type::size() / packed_page; length < bucket.second()) {
            auto &allocator = bucket.first();
            auto &len = bucket.second();
            alloc_ptr allocator_ptr{allocator};

            const auto mem = alloc_ptr_traits::allocate(allocator_ptr, length);
            std::uninitialized_copy(packed, packed + length, mem);

            for(auto pos = length; pos < len; ++pos) {
                alloc_traits::deallocate(allocator, packed[pos], packed_page);
            }

            std::destroy(packed, packed + len);
            alloc_ptr_traits::deallocate(allocator_ptr, packed, len);

            packed = mem;
            len = length;
        }
    }

    template<typename... Args>
    void construct(alloc_pointer ptr, Args &&... args) {
        if constexpr(std::is_aggregate_v<value_type>) {
            alloc_traits::construct(bucket.first(), to_address(ptr), Type{std::forward<Args>(args)...});
        } else {
            alloc_traits::construct(bucket.first(), to_address(ptr), std::forward<Args>(args)...);
        }
    }

    void destroy(Type &elem) {
        alloc_traits::destroy(bucket.first(), std::addressof(elem));
    }

protected:
    /**
     * @brief Exchanges the contents with those of a given storage.
     * @param base Reference to base storage to exchange the content with.
     */
    void swap_contents(underlying_type &base) override {
        using std::swap;
        auto &other = static_cast<basic_storage &>(base);
        propagate_on_container_swap(bucket.first(), other.bucket.first());
        swap(bucket.second(), other.bucket.second());
        swap(packed, other.packed);
    }

    /**
     * @brief Swaps two elements in the internal packed array.
     * @param lhs A valid position of an element within storage.
     * @param rhs A valid position of an element within storage.
     */
    void swap_at(const std::size_t lhs, const std::size_t rhs) final {
        std::swap(packed[page(lhs)][offset(lhs)], packed[page(rhs)][offset(rhs)]);
    }

    /**
     * @brief Moves an element in the internal packed array.
     * @param from A valid position of an element within storage.
     * @param to A valid position of an element within storage.
     */
    void move_and_pop(const std::size_t from, const std::size_t to) final {
        auto &&elem = packed[page(from)][offset(from)];
        construct(packed[page(to)] + offset(to), std::move(elem));
        destroy(elem);
    }

    /**
     * @brief Attempts to erase an element from the internal packed array.
     * @param entt A valid identifier.
     * @param ud Optional user data that are forwarded as-is to derived classes.
     */
    void swap_and_pop(const Entity entt, void *ud) override {
        const auto pos = base_type::index(entt);
        const auto last = base_type::size() - 1u;

        auto &&target = packed[page(pos)][offset(pos)];
        auto &&elem = packed[page(last)][offset(last)];

        // support for nosy destructors
        [[maybe_unused]] auto unused = std::move(target);
        target = std::move(elem);
        destroy(elem);

        base_type::swap_and_pop(entt, ud);
    }

    /**
     * @brief Erases an element from the internal packed array.
     * @param entt A valid identifier.
     * @param ud Optional user data that are forwarded as-is to derived classes.
     */
    void in_place_pop(const Entity entt, void *ud) override {
        const auto pos = base_type::index(entt);
        base_type::in_place_pop(entt, ud);
        // support for nosy destructors
        destroy(packed[page(pos)][offset(pos)]);
    }

public:
    /*! @brief Base type. */
    using base_type = underlying_type;
    /*! @brief Allocator type. */
    using allocator_type = Allocator;
    /*! @brief Type of the objects assigned to entities. */
    using value_type = Type;
    /*! @brief Underlying entity identifier. */
    using entity_type = Entity;
    /*! @brief Unsigned integer type. */
    using size_type = std::size_t;
    /*! @brief Pointer type to contained elements. */
    using pointer = alloc_ptr_pointer;
    /*! @brief Constant pointer type to contained elements. */
    using const_pointer = alloc_ptr_const_pointer;
    /*! @brief Random access iterator type. */
    using iterator = storage_iterator<value_type>;
    /*! @brief Constant random access iterator type. */
    using const_iterator = storage_iterator<const value_type>;
    /*! @brief Reverse iterator type. */
    using reverse_iterator = std::reverse_iterator<iterator>;
    /*! @brief Constant reverse iterator type. */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /*! @brief Default constructor. */
    basic_storage()
        : base_type{deletion_policy{comp_traits::in_place_delete::value}},
          bucket{allocator_type{}, size_type{}},
          packed{}
    {}

    /**
     * @brief Constructs an empty storage with a given allocator.
     * @param allocator The allocator to use.
     */
    explicit basic_storage(const allocator_type &allocator)
        : base_type{deletion_policy{comp_traits::in_place_delete::value}, allocator},
          bucket{allocator, size_type{}},
          packed{}
    {}

    /**
     * @brief Move constructor.
     * @param other The instance to move from.
     */
    basic_storage(basic_storage &&other) ENTT_NOEXCEPT
        : base_type{std::move(other)},
          bucket{std::move(other.bucket.first()), std::exchange(other.bucket.second(), size_type{})},
          packed{std::exchange(other.packed, alloc_ptr_pointer{})}
    {}

    /**
     * @brief Allocator-extended move constructor.
     * @param other The instance to move from.
     * @param allocator The allocator to use.
     */
    basic_storage(basic_storage &&other, const allocator_type &allocator) ENTT_NOEXCEPT
        : base_type{std::move(other), allocator},
          bucket{allocator, std::exchange(other.bucket.second(), size_type{})},
          packed{std::exchange(other.packed, alloc_ptr_pointer{})}
    {
        ENTT_ASSERT(alloc_traits::is_always_equal::value || bucket.first() == other.bucket.first(), "Copying a storage is not allowed");
    }

    /*! @brief Default destructor. */
    ~basic_storage() override {
        release_memory();
    }

    /**
     * @brief Move assignment operator.
     * @param other The instance to move from.
     * @return This storage.
     */
    basic_storage & operator=(basic_storage &&other) ENTT_NOEXCEPT {
        release_memory();
        base_type::operator=(std::move(other));
        propagate_on_container_move_assignment(bucket.first(), other.bucket.first());
        bucket.second() = std::exchange(other.bucket.second(), size_type{});
        packed = std::exchange(other.packed, alloc_ptr_pointer{});
        return *this;
    }

    /**
     * @brief Returns the associated allocator.
     * @return The associated allocator.
     */
    [[nodiscard]] constexpr allocator_type get_allocator() const ENTT_NOEXCEPT {
        return allocator_type{bucket.first()};
    }

    /**
     * @brief Increases the capacity of a storage.
     *
     * If the new capacity is greater than the current capacity, new storage is
     * allocated, otherwise the method does nothing.
     *
     * @param cap Desired capacity.
     */
    void reserve(const size_type cap) override {
        base_type::reserve(cap);

        if(cap > base_type::size()) {
            assure_at_least(cap - 1u);
        }
    }

    /**
     * @brief Returns the number of elements that a storage has currently
     * allocated space for.
     * @return Capacity of the storage.
     */
    [[nodiscard]] size_type capacity() const ENTT_NOEXCEPT override {
        return bucket.second() * packed_page;
    }

    /*! @brief Requests the removal of unused capacity. */
    void shrink_to_fit() override {
        base_type::shrink_to_fit();
        release_unused_pages();
    }

    /**
     * @brief Direct access to the array of objects.
     * @return A pointer to the array of objects.
     */
    [[nodiscard]] const_pointer raw() const ENTT_NOEXCEPT {
        return packed;
    }

    /*! @copydoc raw */
    [[nodiscard]] pointer raw() ENTT_NOEXCEPT {
        return packed;
    }

    /**
     * @brief Returns an iterator to the beginning.
     *
     * The returned iterator points to the first instance of the internal array.
     * If the storage is empty, the returned iterator will be equal to `end()`.
     *
     * @return An iterator to the first instance of the internal array.
     */
    [[nodiscard]] const_iterator cbegin() const ENTT_NOEXCEPT {
        const difference_type pos = base_type::size();
        return const_iterator{std::addressof(packed), pos};
    }

    /*! @copydoc cbegin */
    [[nodiscard]] const_iterator begin() const ENTT_NOEXCEPT {
        return cbegin();
    }

    /*! @copydoc begin */
    [[nodiscard]] iterator begin() ENTT_NOEXCEPT {
        const difference_type pos = base_type::size();
        return iterator{std::addressof(packed), pos};
    }

    /**
     * @brief Returns an iterator to the end.
     *
     * The returned iterator points to the element following the last instance
     * of the internal array. Attempting to dereference the returned iterator
     * results in undefined behavior.
     *
     * @return An iterator to the element following the last instance of the
     * internal array.
     */
    [[nodiscard]] const_iterator cend() const ENTT_NOEXCEPT {
        return const_iterator{std::addressof(packed), {}};
    }

    /*! @copydoc cend */
    [[nodiscard]] const_iterator end() const ENTT_NOEXCEPT {
        return cend();
    }

    /*! @copydoc end */
    [[nodiscard]] iterator end() ENTT_NOEXCEPT {
        return iterator{std::addressof(packed), {}};
    }

    /**
     * @brief Returns a reverse iterator to the beginning.
     *
     * The returned iterator points to the first instance of the reversed
     * internal array. If the storage is empty, the returned iterator will be
     * equal to `rend()`.
     *
     * @return An iterator to the first instance of the reversed internal array.
     */
    [[nodiscard]] const_reverse_iterator crbegin() const ENTT_NOEXCEPT {
        return std::make_reverse_iterator(cend());
    }

    /*! @copydoc crbegin */
    [[nodiscard]] const_reverse_iterator rbegin() const ENTT_NOEXCEPT {
        return crbegin();
    }

    /*! @copydoc rbegin */
    [[nodiscard]] reverse_iterator rbegin() ENTT_NOEXCEPT {
        return std::make_reverse_iterator(end());
    }

    /**
     * @brief Returns a reverse iterator to the end.
     *
     * The returned iterator points to the element following the last instance
     * of the reversed internal array. Attempting to dereference the returned
     * iterator results in undefined behavior.
     *
     * @return An iterator to the element following the last instance of the
     * reversed internal array.
     */
    [[nodiscard]] const_reverse_iterator crend() const ENTT_NOEXCEPT {
        return std::make_reverse_iterator(cbegin());
    }

    /*! @copydoc crend */
    [[nodiscard]] const_reverse_iterator rend() const ENTT_NOEXCEPT {
        return crend();
    }

    /*! @copydoc rend */
    [[nodiscard]] reverse_iterator rend() ENTT_NOEXCEPT {
        return std::make_reverse_iterator(begin());
    }

    /**
     * @brief Returns the object assigned to an entity.
     *
     * @warning
     * Attempting to use an entity that doesn't belong to the storage results in
     * undefined behavior.
     *
     * @param entt A valid identifier.
     * @return The object assigned to the entity.
     */
    [[nodiscard]] const value_type & get(const entity_type entt) const ENTT_NOEXCEPT {
        const auto idx = base_type::index(entt);
        return packed[page(idx)][offset(idx)];
    }

    /*! @copydoc get */
    [[nodiscard]] value_type & get(const entity_type entt) ENTT_NOEXCEPT {
        return const_cast<value_type &>(std::as_const(*this).get(entt));
    }

    /**
     * @brief Returns the object assigned to an entity as a tuple.
     *
     * @sa get
     *
     * @param entt A valid identifier.
     * @return The object assigned to the entity as a tuple.
     */
    [[nodiscard]] std::tuple<const value_type &> get_as_tuple(const entity_type entt) const ENTT_NOEXCEPT {
        return std::forward_as_tuple(get(entt));
    }

    /*! @copydoc get_as_tuple */
    [[nodiscard]] std::tuple<value_type &> get_as_tuple(const entity_type entt) ENTT_NOEXCEPT {
        return std::forward_as_tuple(get(entt));
    }

    /**
     * @brief Assigns an entity to a storage and constructs its object.
     *
     * This version accept both types that can be constructed in place directly
     * and types like aggregates that do not work well with a placement new as
     * performed usually under the hood during an _emplace back_.
     *
     * @warning
     * Attempting to use an entity that already belongs to the storage results
     * in undefined behavior.
     *
     * @tparam Args Types of arguments to use to construct the object.
     * @param entt A valid identifier.
     * @param args Parameters to use to construct an object for the entity.
     * @return A reference to the newly created object.
     */
    template<typename... Args>
    value_type & emplace(const entity_type entt, Args &&... args) {
        const auto pos = base_type::slot();
        assure_at_least(pos);

        alloc_pointer elem = packed[page(pos)] + offset(pos);
        construct(elem, std::forward<Args>(args)...);

        ENTT_TRY {
            [[maybe_unused]] const auto curr = base_type::emplace(entt);
            ENTT_ASSERT(pos == curr, "Misplaced component");
        } ENTT_CATCH {
            destroy(packed[page(pos)][offset(pos)]);
            ENTT_THROW;
        }

        return *elem;
    }

    /**
     * @brief Updates the instance assigned to a given entity in-place.
     * @tparam Func Types of the function objects to invoke.
     * @param entt A valid identifier.
     * @param func Valid function objects.
     * @return A reference to the updated instance.
     */
    template<typename... Func>
    value_type & patch(const entity_type entt, Func &&... func) {
        const auto idx = base_type::index(entt);
        auto &&elem = packed[page(idx)][offset(idx)];
        (std::forward<Func>(func)(elem), ...);
        return elem;
    }

    /**
     * @brief Assigns one or more entities to a storage and constructs their
     * objects from a given instance.
     *
     * @warning
     * Attempting to assign an entity that already belongs to the storage
     * results in undefined behavior.
     *
     * @tparam It Type of input iterator.
     * @param first An iterator to the first element of the range of entities.
     * @param last An iterator past the last element of the range of entities.
     * @param value An instance of the object to construct.
     */
    template<typename It>
    void insert(It first, It last, const value_type &value = {}) {
        reserve(base_type::size() + std::distance(first, last));

        for(; first != last; ++first) {
            const auto pos = base_type::size();
            construct(packed[page(pos)] + offset(pos), value);

            ENTT_TRY {
                base_type::emplace_back(*first);
            } ENTT_CATCH {
                destroy(packed[page(pos)][offset(pos)]);
                ENTT_THROW;
            }
        }
    }

    /**
     * @brief Assigns one or more entities to a storage and constructs their
     * objects from a given range.
     *
     * @sa construct
     *
     * @tparam EIt Type of input iterator.
     * @tparam CIt Type of input iterator.
     * @param first An iterator to the first element of the range of entities.
     * @param last An iterator past the last element of the range of entities.
     * @param from An iterator to the first element of the range of objects.
     */
    template<typename EIt, typename CIt, typename = std::enable_if_t<std::is_same_v<std::decay_t<typename std::iterator_traits<CIt>::value_type>, value_type>>>
    void insert(EIt first, EIt last, CIt from) {
        reserve(base_type::size() + std::distance(first, last));

        for(; first != last; ++first, ++from) {
            const auto pos = base_type::size();
            construct(packed[page(pos)] + offset(pos), *from);

            ENTT_TRY {
                base_type::emplace_back(*first);
            } ENTT_CATCH {
                destroy(packed[page(pos)][offset(pos)]);
                ENTT_THROW;
            }
        }
    }

private:
    compressed_pair<alloc, size_type> bucket;
    alloc_ptr_pointer packed;
};


/*! @copydoc basic_storage */
template<typename Entity, typename Type, typename Allocator>
class basic_storage<Entity, Type, Allocator, std::enable_if_t<ignore_as_empty_v<Type>>>
    : public basic_sparse_set<Entity, typename std::allocator_traits<Allocator>::template rebind_alloc<Entity>>
{
    using allocator_traits = std::allocator_traits<Allocator>;
    using comp_traits = component_traits<Type>;

public:
    /*! @brief Base type. */
    using base_type = basic_sparse_set<Entity, typename allocator_traits::template rebind_alloc<Entity>>;
    /*! @brief Allocator type. */
    using allocator_type = Allocator;
    /*! @brief Type of the objects assigned to entities. */
    using value_type = Type;
    /*! @brief Underlying entity identifier. */
    using entity_type = Entity;
    /*! @brief Unsigned integer type. */
    using size_type = std::size_t;

    /*! @brief Default constructor. */
    basic_storage()
        : base_type{deletion_policy{comp_traits::in_place_delete::value}}
    {}

    /**
     * @brief Constructs an empty container with a given allocator.
     * @param allocator The allocator to use.
     */
    explicit basic_storage(const allocator_type &allocator)
        : base_type{deletion_policy{comp_traits::in_place_delete::value}, allocator}
    {}

    /**
     * @brief Move constructor.
     * @param other The instance to move from.
     */
    basic_storage(basic_storage &&other) ENTT_NOEXCEPT = default;

    /**
     * @brief Move assignment operator.
     * @param other The instance to move from.
     * @return This storage.
     */
    basic_storage & operator=(basic_storage &&other) ENTT_NOEXCEPT = default;

    /**
     * @brief Returns the associated allocator.
     * @return The associated allocator.
     */
    [[nodiscard]] constexpr allocator_type get_allocator() const ENTT_NOEXCEPT {
        return allocator_type{base_type::get_allocator()};
    }

    /**
     * @brief Returns an empty tuple.
     *
     * @warning
     * Attempting to use an entity that doesn't belong to the storage results in
     * undefined behavior.
     *
     * @param entt A valid identifier.
     * @return Returns an empty tuple.
     */
    [[nodiscard]] std::tuple<> get_as_tuple([[maybe_unused]] const entity_type entt) const ENTT_NOEXCEPT {
        ENTT_ASSERT(base_type::contains(entt), "Storage does not contain entity");
        return std::tuple{};
    }

    /**
     * @brief Assigns an entity to a storage and constructs its object.
     *
     * @warning
     * Attempting to use an entity that already belongs to the storage results
     * in undefined behavior.
     *
     * @tparam Args Types of arguments to use to construct the object.
     * @param entt A valid identifier.
     * @param args Parameters to use to construct an object for the entity.
     */
    template<typename... Args>
    void emplace(const entity_type entt, Args &&... args) {
        [[maybe_unused]] value_type elem{std::forward<Args>(args)...};
        base_type::emplace(entt);
    }

    /**
    * @brief Updates the instance assigned to a given entity in-place.
    * @tparam Func Types of the function objects to invoke.
    * @param entt A valid identifier.
    * @param func Valid function objects.
    */
    template<typename... Func>
    void patch([[maybe_unused]] const entity_type entt, Func &&... func) {
        ENTT_ASSERT(base_type::contains(entt), "Storage does not contain entity");
        (std::forward<Func>(func)(), ...);
    }

    /**
     * @brief Assigns one or more entities to a storage.
     *
     * @warning
     * Attempting to assign an entity that already belongs to the storage
     * results in undefined behavior.
     *
     * @tparam It Type of input iterator.
     * @param first An iterator to the first element of the range of entities.
     * @param last An iterator past the last element of the range of entities.
     */
    template<typename It>
    void insert(It first, It last, const value_type & = {}) {
        base_type::insert(first, last);
    }
};


/**
 * @brief Mixin type to use to wrap basic storage classes.
 * @tparam Type The type of the underlying storage.
 */
template<typename Type>
struct storage_adapter_mixin: Type {
    static_assert(std::is_same_v<typename Type::value_type, std::decay_t<typename Type::value_type>>, "Invalid object type");

    /*! @brief Type of the objects assigned to entities. */
    using value_type = typename Type::value_type;
    /*! @brief Underlying entity identifier. */
    using entity_type = typename Type::entity_type;

    /*! @brief Inherited constructors. */
    using Type::Type;

    /**
     * @brief Assigns entities to a storage.
     * @tparam Args Types of arguments to use to construct the object.
     * @param entt A valid identifier.
     * @param args Parameters to use to initialize the object.
     * @return A reference to the newly created object.
     */
    template<typename... Args>
    decltype(auto) emplace(basic_registry<entity_type> &, const entity_type entt, Args &&... args) {
        return Type::emplace(entt, std::forward<Args>(args)...);
    }

    /**
     * @brief Assigns entities to a storage.
     * @tparam It Type of input iterator.
     * @tparam Args Types of arguments to use to construct the objects assigned
     * to the entities.
     * @param first An iterator to the first element of the range of entities.
     * @param last An iterator past the last element of the range of entities.
     * @param args Parameters to use to initialize the objects assigned to the
     * entities.
     */
    template<typename It, typename... Args>
    void insert(basic_registry<entity_type> &, It first, It last, Args &&... args) {
        Type::insert(first, last, std::forward<Args>(args)...);
    }

    /**
     * @brief Patches the given instance for an entity.
     * @tparam Func Types of the function objects to invoke.
     * @param entt A valid identifier.
     * @param func Valid function objects.
     * @return A reference to the patched instance.
     */
    template<typename... Func>
    decltype(auto) patch(basic_registry<entity_type> &, const entity_type entt, Func &&... func) {
        return Type::patch(entt, std::forward<Func>(func)...);
    }
};


/**
 * @brief Mixin type to use to add signal support to storage types.
 * @tparam Type The type of the underlying storage.
 */
template<typename Type>
class sigh_storage_mixin final: public Type {
    /*! @copydoc basic_sparse_set::swap_and_pop */
    void swap_and_pop(const typename Type::entity_type entt, void *ud) final {
        ENTT_ASSERT(ud != nullptr, "Invalid pointer to registry");
        destruction.publish(*static_cast<basic_registry<typename Type::entity_type> *>(ud), entt);
        Type::swap_and_pop(entt, ud);
    }

    /*! @copydoc basic_sparse_set::in_place_pop */
    void in_place_pop(const typename Type::entity_type entt, void *ud) final {
        ENTT_ASSERT(ud != nullptr, "Invalid pointer to registry");
        destruction.publish(*static_cast<basic_registry<typename Type::entity_type> *>(ud), entt);
        Type::in_place_pop(entt, ud);
    }

public:
    /*! @brief Underlying value type. */
    using value_type = typename Type::value_type;
    /*! @brief Underlying entity identifier. */
    using entity_type = typename Type::entity_type;

    /*! @brief Inherited constructors. */
    using Type::Type;

    /**
     * @brief Returns a sink object.
     *
     * The sink returned by this function can be used to receive notifications
     * whenever a new instance is created and assigned to an entity.<br/>
     * The function type for a listener is equivalent to:
     *
     * @code{.cpp}
     * void(basic_registry<entity_type> &, entity_type);
     * @endcode
     *
     * Listeners are invoked **after** the object has been assigned to the
     * entity.
     *
     * @sa sink
     *
     * @return A temporary sink object.
     */
    [[nodiscard]] auto on_construct() ENTT_NOEXCEPT {
        return sink{construction};
    }

    /**
     * @brief Returns a sink object.
     *
     * The sink returned by this function can be used to receive notifications
     * whenever an instance is explicitly updated.<br/>
     * The function type for a listener is equivalent to:
     *
     * @code{.cpp}
     * void(basic_registry<entity_type> &, entity_type);
     * @endcode
     *
     * Listeners are invoked **after** the object has been updated.
     *
     * @sa sink
     *
     * @return A temporary sink object.
     */
    [[nodiscard]] auto on_update() ENTT_NOEXCEPT {
        return sink{update};
    }

    /**
     * @brief Returns a sink object.
     *
     * The sink returned by this function can be used to receive notifications
     * whenever an instance is removed from an entity and thus destroyed.<br/>
     * The function type for a listener is equivalent to:
     *
     * @code{.cpp}
     * void(basic_registry<entity_type> &, entity_type);
     * @endcode
     *
     * Listeners are invoked **before** the object has been removed from the
     * entity.
     *
     * @sa sink
     *
     * @return A temporary sink object.
     */
    [[nodiscard]] auto on_destroy() ENTT_NOEXCEPT {
        return sink{destruction};
    }

    /**
     * @brief Assigns entities to a storage.
     * @tparam Args Types of arguments to use to construct the object.
     * @param owner The registry that issued the request.
     * @param entt A valid identifier.
     * @param args Parameters to use to initialize the object.
     * @return A reference to the newly created object.
     */
    template<typename... Args>
    decltype(auto) emplace(basic_registry<entity_type> &owner, const entity_type entt, Args &&... args) {
        Type::emplace(entt, std::forward<Args>(args)...);
        construction.publish(owner, entt);

        if constexpr(!ignore_as_empty_v<value_type>) {
            return this->get(entt);
        }
    }

    /**
     * @brief Assigns entities to a storage.
     * @tparam It Type of input iterator.
     * @tparam Args Types of arguments to use to construct the objects assigned
     * to the entities.
     * @param owner The registry that issued the request.
     * @param first An iterator to the first element of the range of entities.
     * @param last An iterator past the last element of the range of entities.
     * @param args Parameters to use to initialize the objects assigned to the
     * entities.
     */
    template<typename It, typename... Args>
    void insert(basic_registry<entity_type> &owner, It first, It last, Args &&... args) {
        Type::insert(first, last, std::forward<Args>(args)...);

        if(!construction.empty()) {
            for(; first != last; ++first) {
                construction.publish(owner, *first);
            }
        }
    }

    /**
     * @brief Patches the given instance for an entity.
     * @tparam Func Types of the function objects to invoke.
     * @param owner The registry that issued the request.
     * @param entt A valid identifier.
     * @param func Valid function objects.
     * @return A reference to the patched instance.
     */
    template<typename... Func>
    decltype(auto) patch(basic_registry<entity_type> &owner, const entity_type entt, Func &&... func) {
        Type::patch(entt, std::forward<Func>(func)...);
        update.publish(owner, entt);

        if constexpr(!ignore_as_empty_v<value_type>) {
            return this->get(entt);
        }
    }

private:
    sigh<void(basic_registry<entity_type> &, const entity_type)> construction{};
    sigh<void(basic_registry<entity_type> &, const entity_type)> destruction{};
    sigh<void(basic_registry<entity_type> &, const entity_type)> update{};
};


/**
 * @brief Provides a common way to access certain properties of storage types.
 * @tparam Entity A valid entity type (see entt_traits for more details).
 * @tparam Type Type of objects managed by the storage class.
 */
template<typename Entity, typename Type, typename = void>
struct storage_traits {
    /*! @brief Resulting type after component-to-storage conversion. */
    using storage_type = sigh_storage_mixin<basic_storage<Entity, Type>>;
};


}


#endif
