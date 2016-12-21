#pragma once
#include "lhelper.hpp"
#include <type_traits>

namespace eds
{
    // refer to a block of memory with no awareness if its content is initialized
    // as a result, this class offer no direct access to data
    template <typename TElem>
    class ArrayRef
    {
    public:
        ArrayRef() : data_(nullptr), size_(0) { }
        ArrayRef(TElem *p, size_t sz)
            : data_(p), size_(sz) 
        { 
            Expects(p != nullptr && sz != 0);
        }

    public:
        TElem *FrontPointer() noexcept
        {
            return data_;
        }
        const TElem *FrontPointer() const noexcept
        {
            return data_;
        }
        TElem *BackPointer() noexcept
        {
            return data_ + size_;
        }
        const TElem *BackPointer() const noexcept
        {
            return data_ + size_;
        }

        size_t Size() const noexcept
        {
            return size_;
        }

        bool Empty() const noexcept
        {
            return data_ == nullptr;
        }

    private:
        size_t size_;
        TElem *data_;
    };

    class AdapterBoundaryError : std::runtime_error
    {
    public:
        AdapterBoundaryError(const char *msg)
            : std::runtime_error(msg) { }
    };

    template <typename TElem>
    class VectorAdapter : Uncopyable
    {
        static_assert(std::is_trivially_destructible<TElem>::value, "!!!");
    public:
        VectorAdapter() = default;
        VectorAdapter(ArrayRef<TElem> buffer)
            : buffer_(buffer) { }
        VectorAdapter(ArrayRef<TElem> buffer, std::initializer_list<TElem> ilist)
            : buffer_(buffer)
        {
            Expects(buffer.Size() >= ilist.size());
            for (const TElem &x : ilist)
            {
                EmplaceBack(x);
            }
        }

        VectorAdapter(VectorAdapter &&other)
            : buffer_(other.buffer_), size_(other.size_)
        {
            other.buffer_ = ArrayRef<TElem>{ };
            other.size_ = 0;
        }

    public:
        void PushBack(const TElem &value)
        {
            EmplaceBack(value);
        }
        void PushBack(TElem &&value)
        {
            EmplaceBack(std::move(value));
        }

        template <typename ... TArgs>
        void EmplaceBack(TArgs&& ...args)
        {
            TestNotFull();

            // NOTE TElem must be trivially destructible
            new (buffer_.FrontPointer() + size_) TElem(std::forward<TArgs>(args)...);
            size_ += 1;
        }

        void PopBack()
        {
            TestNotEmpty();

            // NOTE TElem must be trivially destructible
            size_ -= 1;
        }

        void ShiftTo(ArrayRef<TElem> new_array)
        {
            Asserts(new_array.Size() >= Size());

            // if any data in this vector
            // move them into new_array
            if (!buffer_.Empty() && Size() > 0)
            {
                TElem *begin = buffer_.FrontPointer();
                std::move(begin, begin + Size(), new_array.FrontPointer());
            }

            // NOTE size_ is unchanged
            buffer_ = new_array;
        }

        void Clear()
        {
            size_ = 0;
        }

        size_t Capacity() const noexcept
        {
            return buffer_.Size();
        }

        size_t Size() const noexcept
        {
            return size_;
        }

        bool Empty() const noexcept
        {
            return Size() == 0;
        }

        bool Full() const noexcept
        {
            return Size() == Capacity();
        }
    public:
        TElem *begin() noexcept
        {
            return buffer_.FrontPointer();
        }
        const TElem *begin() const noexcept
        {
            return buffer_.FrontPointer();
        }
        TElem *end() noexcept
        {
            return buffer_.FrontPointer() + size_;
        }
        const TElem *end() const noexcept
        {
            return buffer_.FrontPointer() + size_;
        }


    public:
        TElem &operator[](size_t index)
        {
            Expects(index < Size());
            return *(buffer_.FrontPointer() + index);
        }

    private:
        void TestNotEmpty() const
        {
            if (Empty())
            {
                throw AdapterBoundaryError("adapter is already empty");
            }
        }
        void TestNotFull()
        {
            if (Full())
            {
                throw AdapterBoundaryError("adapter is already full");
            }
        }
    private:
        ArrayRef<TElem> buffer_;
        size_t size_ = 0;
    };

    template <typename TElem>
    class DequeAdapter : Uncopyable
    {
        static_assert(std::is_trivially_destructible<TElem>::value, "!!!");
    public:
        DequeAdapter() = default;
        DequeAdapter(ArrayRef<TElem> buffer)
            : buffer_(buffer) { }
        DequeAdapter(ArrayRef<TElem> buffer, std::initializer_list<TElem> ilist)
            : buffer_(buffer)
        {
            Expects(buffer.Size() >= ilist.size());
            for (const TElem &x : ilist)
            {
                EmplaceBack(x);
            }
        }

        DequeAdapter(DequeAdapter &&other)
            : buffer_(other.buffer_), head_(other.head_), tail_(other.tail_)
        {
            other.buffer_ = ArrayRef<TElem>{};
            other.head_ = 0;
            other.tail_ = 0;
        }

    public:
        void PushFront(const TElem &value)
        {
            EmplaceFront(value);
        }
        void PushFront(TElem &&value)
        {
            EmplaceFront(std::move(value));
        }
        void PushBack(const TElem &value)
        {
            EmplaceBack(value);
        }
        void PushBack(TElem &&value)
        {
            EmplaceBack(std::move(value));
        }

        template <typename ... TArgs>
        void EmplaceFront(TArgs&& ...args)
        {
            TestNotFull();

            size_t new_head = (Capacity() + head_ - 1) % Capacity();
            new (buffer_.FrontPointer() + head_) TElem(std::forward<TArgs>(args)...);

            head_ = new_head;
        }
        template <typename ... TArgs>
        void EmplaceBack(TArgs&& ...args)
        {
            TestNotFull();

            size_t new_tail = (tail_ + 1) % Capacity();
            new (buffer_.FrontPointer() + tail_) TElem(std::forward<TArgs>(args)...);
            
            tail_ = new_tail;
        }

        void PopFront()
        {
            TestNotEmpty();

            // NOTE TElem must be trivially destructible
            head_ = (head_ + 1) % Capacity();
        }
        void PopBack()
        {
            TestNotEmpty();

            // NOTE TElem must be trivially destructible
            tail_ = (Capacity() + tail_ - 1) % Capacity();
        }

        void ShiftTo(ArrayRef<TElem> new_array)
        {
            Asserts(new_array.Size() >= Size());

            // if any data in this deque
            // move them into new_array
            if (!buffer_.Empty() && Size() > 0)
            {
                TElem *begin = buffer_.FrontPointer();
                if (tail_ > head_)
                {
                    std::move(begin + head_, begin + tail_, new_array.FrontPointer());
                }
                else
                {
                    std::move(begin + head_, begin + Capacity(), new_array.FrontPointer());
                    std::move(begin, begin + tail_, new_array.FrontPointer() + Capacity() - head_);
                }

                size_t sz = Size();
                head_ = 0;
                tail_ = sz;
            }

            buffer_ = new_array;
        }

        void Clear()
        {
            head_ = tail_ = 0;
        }

        size_t Capacity() const noexcept
        {
            return buffer_.Size();
        }

        size_t Size() const noexcept
        {
            return tail_ > head_
                ? tail_ - head_
                : Capacity() + tail_ - head_;
        }

        bool Empty() const noexcept
        {
            return Size() == 0;
        }

        bool Full() const noexcept
        {
            return Size() == Capacity();
        }
    private:
    void TestNotEmpty() const
    {
        if (Empty())
        {
            throw AdapterBoundaryError("adapter is already empty");
        }
    }
    void TestNotFull()
    {
        if (Full())
        {
            throw AdapterBoundaryError("adapter is already full");
        }
    }

    private:
        ArrayRef<TElem> buffer_;
        size_t head_ = 0;
        size_t tail_ = 0;
    };
}