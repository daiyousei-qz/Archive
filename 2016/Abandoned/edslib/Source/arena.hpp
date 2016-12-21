/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

#pragma once
#include "lhelper.hpp"
#include "unsafe_container.hpp"
#include <type_traits>

namespace eds
{
    ///<summary>
    /// A guard object for chunks of trivial memory, free the memory when destructed.
    /// THIS IS NOT A MEMORY POOL!
    ///</summary>
    class Arena final : Uncopyable
    {
    private:
        struct Block
        {
            Block *next;
            size_t size;
            size_t offset;
            size_t counter;

            char *DataAddress()
            {
                return reinterpret_cast<char*>(this + 1);
            }
        };

        static constexpr size_t kDefaultAlignment = alignof(nullptr_t);
        static constexpr size_t kFailureToleranceCount = 8;
        static constexpr size_t kFailureCounterThreshold = 1024;
        static constexpr size_t kBigChunkThreshold = 2048;
        static constexpr size_t kMemoryPoolStoreSize = 4096 - sizeof(Block);

    public:
        Arena()
            : pooled_head_(nullptr)
            , pooled_current_(nullptr)
            , big_node_(nullptr) { }

        Arena(Arena &&other)
            : pooled_head_(other.pooled_head_)
            , pooled_current_(other.pooled_current_)
            , big_node_(other.big_node_)
        {
            other.pooled_head_ = nullptr;
            other.pooled_current_ = nullptr;
            other.big_node_ = nullptr;
        }
        Arena &operator=(Arena &&rhs)
        {
            // move fields from rhs to *this
            pooled_head_ = rhs.pooled_head_;
            pooled_current_ = rhs.pooled_current_;
            big_node_ = rhs.big_node_;

            // clear rhs
            rhs.pooled_head_ = nullptr;
            rhs.pooled_current_ = nullptr;
            rhs.big_node_ = nullptr;
        }

        ~Arena()
        {
            FreeBlocks(pooled_head_);
            FreeBlocks(big_node_);
        }

    public:
        void *Allocate(size_t sz) const
        {
            // ensure aligned
            if (sz % kDefaultAlignment)
            {
                sz += kDefaultAlignment - sz % kDefaultAlignment;
            }

            if (IsBigChunk(sz))
            {
                // big chunk of memory should be allocated directly from allocater
                return AllocBigChunk(sz);
            }
            else
            {
                // small chunk should be allocated from the internal pool
                return AllocSmallChunk(sz);
            }
        }

        template <typename T>
        T* Allocate() const
        {
            return reinterpret_cast<T*>(Allocate(sizeof(T)));
        }

        template <typename T>
        ArrayRef<T> Allocate(size_t count) const
        {
            Expects(count > 0);
            T* p = reinterpret_cast<T*>(Allocate(sizeof(T) * count));

            return ArrayRef<T>{ p, count };
        }

        template <typename T, typename ...TArgs>
        T *ConstructUnchecked(TArgs&& ...args) const
        {
            T *p = Allocate<T>();
            new (p) T(std::forward<TArgs>(args)...);

            return p;
        }

        template <typename T, typename ...TArgs>
        T *Construct(TArgs&& ...args) const
        {
            static_assert(std::is_trivially_destructible<T>::value, "!!!");

            return ConstructUnchecked<T>(std::forward<TArgs>(args)...);
        }

        size_t GetByteAllocated() const
        {
            return CalcUsage(pooled_head_, false) + CalcUsage(big_node_, false);
        }

        size_t GetByteUsed() const
        {
            return CalcUsage(pooled_head_, true) + CalcUsage(big_node_, true);
        }

    private:
        bool IsBigChunk(size_t sz) const noexcept
        {
            return sz > kBigChunkThreshold;
        }

        Block *NewBlock(size_t capacity) const
        {
            // allocate memory
            void *p = malloc(sizeof(Block) + capacity);
            Block *block = reinterpret_cast<Block*>(p);

            // initialize block
            block->next = nullptr;
            block->size = capacity;
            block->offset = 0;
            block->counter = 0;

            return block;
        }

        void FreeBlocks(Block *list) const
        {
            Block *p = list;
            while (p != nullptr)
            {
                // store pointer to the next block before free the current
                Block *next = p->next;

                free(p);
                p = next;
            }
        }

        size_t CalcUsage(Block *list, bool used) const
        {
            size_t sum = 0;
            for (Block *p = list; p != nullptr; p = p->next)
            {
                if (used)
                {
                    sum += p->offset;
                }
                else
                {
                    sum += p->size;
                }
            }

            return sum;
        }

        void *AllocSmallChunk(size_t sz) const
        {
            Asserts(!IsBigChunk(sz));

            // lazy initialization
            if (pooled_current_ == nullptr)
            {
                pooled_head_ = pooled_current_ = NewBlock(kMemoryPoolStoreSize);
            }

            // find a chunk of memory in the memory pool
            Block *cur = pooled_current_;
            while (true)
            {
                size_t available_sz = cur->size - cur->offset;
                if (available_sz >= sz)
                {
                    // enough memory in the current Block
                    void *addr = cur->DataAddress() + cur->offset;
                    cur->offset += sz;
                    return addr;
                }
                else
                {
                    if (available_sz < kFailureCounterThreshold)
                    {
                        // failed to allocate and available size is small enough
                        cur->counter += 1;
                    }

                    // roll to next Block, allocate one if neccessary
                    Block *next = cur->next != nullptr 
                        ? cur->next 
                        : (cur->next = NewBlock(kMemoryPoolStoreSize));
                    // if the current Block has beening failing too much, drop it
                    if (cur->counter > kFailureToleranceCount)
                    {
                        pooled_current_ = next;
                    }

                    cur = next;
                }
            }
        }

        void *AllocBigChunk(size_t sz) const
        {
            Asserts(IsBigChunk(sz))

            // note this works even if big_node_ == nullptr
            Block *cur = NewBlock(sz);
            cur->next = big_node_;
            big_node_ = cur;

            return cur->DataAddress();
        }

    private:
        // blocks for memory pool
        mutable Block *pooled_head_;
        mutable Block *pooled_current_;
        // blocks for big chunk allocation
        mutable Block *big_node_;
    };

}