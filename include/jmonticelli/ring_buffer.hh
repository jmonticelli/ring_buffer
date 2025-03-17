/**
 * Copyright (c) 2025 Julian Monticelli.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
 */

#ifndef INCLUDE_GUARD_JMONTICELLI_RING_BUFFER_HH_
#define INCLUDE_GUARD_JMONTICELLI_RING_BUFFER_HH_

#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>

#include <iostream>

#if defined(__GNUC__) || defined (__clang__)
    #define JMONTICELLI_RING_BUFFER_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define JMONTICELLI_RING_BUFFER_ALWAYS_INLINE __attribute__((always_inline))
#else
    #define JMONTICELLI_RING_BUFFER_ALWAYS_INLINE __forceinline
#endif

namespace jmonticelli {

template <class ValueType,
          class Allocator = std::allocator<ValueType>>
class ring_buffer
{
public:
    using value_type = ValueType;

    ring_buffer() = delete;

    ring_buffer(const std::size_t max_capacity,
                const Allocator& allocator=Allocator())
        : max_capacity_(max_capacity)
        , current_size_(0)
        , allocator_(allocator)
    {
        if (max_capacity_ == 0)
        {
            throw std::runtime_error("ring_buffer: invalid size 0 requested");
        }

        // Overflow seems possible for relative/wraparound indexes
        // if capacity > max/2. To prevent this (lazily), cap the
        // size at std::size_t max / 2.
        if (max_capacity_ >= std::numeric_limits<std::size_t>::max() / static_cast<std::size_t>(2))
        {
            throw std::runtime_error("ring_buffer: exceeded limit std::size_t max / 2");
        }

        if (sizeof(ValueType) == 0)
        {
            throw std::runtime_error("ring_buffer: Cannot allocate sizeless type");
        }

        buffer_ = static_cast<ValueType*>(allocator_.allocate(max_capacity_));
        buffer_end_ = buffer_ + (max_capacity_ - 1);
        begin_ptr_ = buffer_;
        end_ptr_ = buffer_;
        if (nullptr == buffer_)
        {
            throw std::runtime_error("ring_buffer: Failed to allocate internal buffer");
        }
    }

    ring_buffer(const ring_buffer& other)
        : max_capacity_(other.max_capacity_)
        , current_size_(0)
        , allocator_(other.allocator_)
    {
        if (max_capacity_ == 0)
        {
            throw std::runtime_error("ring_buffer: invalid size 0 requested");
        }

        buffer_ = static_cast<ValueType*>(allocator_.allocate(max_capacity_));
        buffer_end_ = buffer_ + (max_capacity_ - 1);
        begin_ptr_ = buffer_;
        end_ptr_ = buffer_;
        if (nullptr == buffer_)
        {
            throw std::runtime_error("ring_buffer: Failed to allocate internal buffer");
        }

        std::copy(other.cbegin(), other.cend(), std::back_inserter(*this));
    }

    ~ring_buffer()
    {
        allocator_.deallocate(buffer_, max_capacity_);;
    }

    void push_back(const ValueType& type)
    {
        add_back_uninitialized_element();
        *end_ptr_ = type;
    }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        add_back_uninitialized_element();
        *end_ptr_ = ValueType(args...);
    }

    void push_front(const ValueType& type)
    {
        add_front_uninitialized_element();
        *begin_ptr_ = type;
    }

    template <class... Args>
    void emplace_front(Args&&... args)
    {
        add_front_uninitialized_element();
        *begin_ptr_ = ValueType(args...);
    }

    ValueType& at(const std::size_t& idx)
    {
        assert_idx(idx, "at");
        return *unchecked_ptr_from_idx(idx);
    }

    const ValueType& at(const std::size_t& idx) const
    {
        assert_idx(idx, "at");
        return *unchecked_ptr_from_idx(idx);
    }


    ValueType& operator[](const std::size_t idx)
    {
        return at(idx);
    }

    const ValueType& operator[](const std::size_t idx) const
    {
        return at(idx);
    }

    ValueType& front()
    {
        return at(0);
    }

    const ValueType& front() const
    {
        return at(0);
    }

    ValueType& back()
    {
        return at(current_size_ == 0 ? 0 : current_size_ - 1);
    }

    /**
     * Returns the
     */
    const ValueType& back() const
    {
        return at(current_size_ == 0 ? 0 : current_size_ - 1);
    }

    /**
     * Removes the front element.
     *
     * \return true if an element was removed.
     */
    bool pop_front()
    {
        if (current_size_ == 0)
        {
            return false;
        }

        begin_ptr_ = remove_advance_right();
        --current_size_;
        return true;
    }

    /**
     * Removes the back element
     *
     * \return true if an element was removed.
     */
    bool pop_back()
    {
        if (current_size_ == 0)
        {
            return false;
        }

        end_ptr_ = remove_advance_left();
        --current_size_;
        return true;
    }

    /**
     * Clears the currently held elements from this ring buffer.
     */
    void clear()
    {
        remove_elements();

        current_size_ = std::size_t{0};
        begin_ptr_ = buffer_;
        end_ptr_ = buffer_;
    }

    /**
     * Returns the current number of elements held in this ring buffer.
     */
    std::size_t size() const noexcept
    {
        return current_size_;
    }

    /**
     * Returns the maximum capacity of the ring_buffer.
     */
    std::size_t capacity() const noexcept
    {
        return max_capacity_;
    }

    template <typename ItValueType = ValueType,
              bool IsConst=false>
    struct internal_Iterator
    {
        public:
            using ContainerType = typename
                std::conditional<IsConst,
                                 const ring_buffer<ItValueType>,
                                 ring_buffer<ItValueType>>::type;

            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type = std::int64_t;
            using value_type = ItValueType;
            using pointer = ItValueType*;
            using reference = ItValueType&;

            internal_Iterator(ContainerType* container, std::size_t idx) noexcept
                : container_(container)
                , idx_(idx)
            {
            }

            bool operator==(const internal_Iterator& other) const
            {
                return this->idx_ == other.idx_;
            }
            bool operator!=(const internal_Iterator& other) const
            {
                return not (*this == other);
            }

            internal_Iterator& operator++()
            {
                idx_ = std::min(container_->size(), idx_ + 1);
                return *this;
            }

            internal_Iterator operator++() const
            {
                auto tmp = (*this);
                return ++tmp;
            }

            internal_Iterator& operator--()
            {
                if (idx_ <= 0)
                {
                    return *this;
                }

                --idx_;
                return *this;
            }

            internal_Iterator operator--() const
            {
                auto tmp = (*this);
                --tmp;
                return tmp;
            }

            template <typename ValueType_ = ItValueType>
            typename std::enable_if<not IsConst, ValueType_&>::type
            operator*()
            {
                return container_->at(idx_);
            }

            const ValueType& operator*() const
            {
                return container_->at(idx_);
            }

            template <typename ValueType_ = ItValueType>
            typename std::enable_if<not IsConst, ValueType_*>::type
            operator->()
            {
                return container_->unchecked_ptr_from_idx(idx_);
            }

            const ValueType* operator->() const
            {
                return container_->unchecked_ptr_from_idx(idx_);
            }

        private:
            ContainerType* container_;
            std::size_t idx_;
    };

    // Iterator definitions
    using Iterator = internal_Iterator<ValueType, false>;
    using ConstIterator = internal_Iterator<ValueType, true>;

    /**
     * Returns an iterator to the beginning of the buffer.
     */
    Iterator begin() noexcept
    {
        return Iterator(this, 0);
    }

    /**
     * Returns a constant iterator to the beginning of the buffer.
     */
    const ConstIterator cbegin() const noexcept
    {
        return ConstIterator(this, 0);
    }

    /**
     * Returns an iterator to an element past the end of the buffer.
     * This iterator is invalid, and should fail to dereference.
     */
    Iterator end() noexcept
    {
        return Iterator(this, current_size_);
    }

    /**
     * Returns a constant iterator to an element past the end of the buffer.
     * This iterator is invalid, and should fail to dereference.
     */
    const ConstIterator cend() const noexcept
    {
        return ConstIterator(this, current_size_);
    }

private:

    /// Asserts the index is valid for a function which specifies its name
    inline void assert_idx(const std::size_t idx, const char* calling_func) const
    {
        if (idx >= current_size_)
        {
            throw std::runtime_error("ring_buffer index out-of-bounds in " + std::string(calling_func));
        }
    }

    /// The unchecked pointer from an index.
    /// This operation can be unsafe without calling assert_idx
    inline ValueType* unchecked_ptr_from_idx(const std::size_t idx) const noexcept
    {
        const auto begin_diff = begin_ptr_ - buffer_;
        const auto buffer_idx = (begin_diff + idx) % max_capacity_;

        return &buffer_[buffer_idx];
    }

    /**
     * Advances the provided pointer in the left direction, appropriately
     * handling wraparound.
     *
     * \param ptr The pointer to advance
     * \return The pointer to the left of the provided pointer in the buffer
     */
    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline ValueType* advance_ptr_left(ValueType* ptr) const noexcept
    {
        if (ptr == buffer_)
        {
            return buffer_end_;
        }
        return --ptr;
    }

    /**
     * Advances the provided pointer in the right direction, appropriately
     * handling wraparound.
     *
     * \param ptr The pointer to advance
     * \return The pointer to the right of the provided pointer in the buffer
     */
    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline ValueType* advance_ptr_right(ValueType* ptr) const noexcept
    {
        if (ptr == buffer_end_)
        {
            return buffer_;
        }
        return ++ptr;
    }

    /**
     * Removes the value held by the pointer at the specified location in the
     * buffer, and advances the pointer left, handling wraparound.
     *
     * This overload calls the destructor, since the type is destructible.
     *
     * \param ptr The pointer whose referenced value shall be removed
     * \return The pointer to the left of the provided pointer in the buffer
     */
    template <typename ValueType_>
    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline typename std::enable_if<std::is_destructible<ValueType_>::value, ValueType_*>::type
    remove_advance_left(ValueType_* ptr) const
    {
        ptr->~ValueType_();
        return advance_ptr_left(ptr);
    }

    /**
     * Removes the value held by the pointer at the specified location in the
     * buffer, and advances the pointer left, handling wraparound.
     *
     * This overload calls no destructor, since the type is not destructible.
     *
     * \param ptr The pointer whose referenced value shall be removed
     * \return The pointer to the left of the provided pointer in the buffer
     */
    template <typename ValueType_>
    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline typename std::enable_if<not std::is_destructible<ValueType_>::value, ValueType_*>::type
    remove_advance_left(ValueType_* ptr) const
    {
        return advance_ptr_left(ptr);
    }

    /**
     * Removes the value held by the pointer at the specified location in the
     * buffer, and advances the pointer right, handling wraparound.
     *
     * This overload calls the destructor, since the type is destructible.
     *
     * \param ptr The pointer whose referenced value shall be removed
     * \return The pointer to the right of the provided pointer in the buffer
     */
    template <typename ValueType_>
    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline typename std::enable_if<std::is_destructible<ValueType_>::value, ValueType_*>::type
    remove_advance_right(ValueType_* ptr) const
    {
        ptr->~ValueType_();
        return advance_ptr_right(ptr);
    }

    /**
     * Removes the value held by the pointer at the specified location in the
     * buffer, and advances the pointer right, handling wraparound.
     *
     * This overload calls no destructor, since the type is not destructible.
     *
     * \param ptr The pointer whose referenced value shall be removed
     * \return The pointer to the right of the provided pointer in the buffer
     */
    template <typename ValueType_>
    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline typename std::enable_if<not std::is_destructible<ValueType_>::value, ValueType_*>::type
    remove_advance_right(ValueType_* ptr) const
    {
        return advance_ptr_right(ptr);
    }

    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline void add_back_uninitialized_element()
    {
        const auto next_size = std::min(current_size_ + 1, max_capacity_);

        if (current_size_ > 0)
        {
            end_ptr_ = advance_ptr_right(end_ptr_);
        }

        if (current_size_ == next_size)
        {
            begin_ptr_ = remove_advance_right(begin_ptr_);
        }
        else
        {
            current_size_ = next_size;
        }
    }

    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline void add_front_uninitialized_element()
    {
        const auto next_size = std::min(current_size_ + 1, max_capacity_);

        if (current_size_ > 0)
        {
            begin_ptr_ = advance_ptr_left(begin_ptr_);
        }

        if (current_size_ == next_size)
        {
            end_ptr_ = remove_advance_left(end_ptr_);
        }
        else
        {
            current_size_ = next_size;
        }
    }

    /**
     * Removes all elements by explicitly calling their destructors.
     */
    template <typename ValueType_ = ValueType>
    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline typename std::enable_if<std::is_destructible<ValueType_>::value, void>::type
    remove_elements()
    {
        for (auto it = begin(); it != end(); ++it)
        {
            it->~ValueType();
        }
    }

    /**
     * A no-op function for removing non-destructible elements.
     */
    template <typename ValueType_ = ValueType>
    JMONTICELLI_RING_BUFFER_ALWAYS_INLINE
    inline typename std::enable_if<not std::is_destructible<ValueType_>::value, void>::type
    remove_elements() noexcept
    {
    }

    const std::size_t max_capacity_;

    Allocator allocator_;

    /// The location of the first element in the absolute buffer
    ValueType* buffer_{nullptr};

    /// The location of the last element in the absolute buffer.
    /// NOTE: This is not a past-the-end pointer.
    ValueType* buffer_end_{nullptr};

    /// The location of the first element in the relativized buffer.
    ValueType* begin_ptr_{nullptr};

    /// The location of the last element in the relativized buffer.
    /// NOTE: This is not a past-the-end pointer.
    ValueType* end_ptr_{nullptr};

    /// The current size of the relativized buffer.
    /// This cannot exceed max_capacity_.
    std::size_t current_size_{0};
};

} // namespace jmonticelli

#undef JMONTICELLI_RING_BUFFER_ALWAYS_INLINE

// close include guard
#endif
