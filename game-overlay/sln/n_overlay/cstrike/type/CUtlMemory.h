// This file modified from https://github.com/alliedmodders/hl2sdk/tree/cs2

#ifndef CSTRIKE_TYPE_CUTLMEMORY_H
#define CSTRIKE_TYPE_CUTLMEMORY_H

#include "global.h"
#include "logging.h"

#include <algorithm>
#include <cstdint>
#include <limits>

enum RawAllocatorType_t : uint8_t
{
    RawAllocator_Standard = 0,
    RawAllocator_Platform = 1,
};

MS_IMPORT int   UtlVectorMemory_CalcNewAllocationCount(int nAllocationCount, int nGrowSize, int nNewSize, int nBytesItem);
MS_IMPORT void* UtlVectorMemory_Alloc(void* pMem, bool bRealloc, int nNewSize, int nOldSize);

template <class T, class I = int32_t>
class CUtlMemory
{
public:
    // constructor, destructor
    CUtlMemory(int nGrowSize = 0, int nInitSize = 0) :
        m_pMemory(nullptr),
        m_nAllocationCount(nInitSize), m_nGrowSize(nGrowSize & ~(EXTERNAL_CONST_BUFFER_MARKER | EXTERNAL_BUFFER_MARKER))
    {
        if (m_nAllocationCount)
        {
            m_pMemory = static_cast<T*>(AllocateMemory(m_nAllocationCount * sizeof(T)));
        }
    }

    CUtlMemory(T* pMemory, int numElements) :
        m_pMemory(pMemory),
        m_nAllocationCount(numElements)
    {
        // Special marker indicating externally supplied modifyable memory
        m_nGrowSize = EXTERNAL_BUFFER_MARKER;
    }

    CUtlMemory(const T* pMemory, int numElements) :
        m_pMemory(pMemory),
        m_nAllocationCount(numElements)
    {
        // Special marker indicating externally supplied modifyable memory
        m_nGrowSize = EXTERNAL_CONST_BUFFER_MARKER;
    }

    ~CUtlMemory()
    {
        Purge();
    }

    using value_type      = T;
    using size_type       = I;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;

    using iterator       = T*;
    using const_iterator = const T*;

    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() noexcept
    {
        return m_pMemory;
    }

    const_iterator begin() const noexcept
    {
        return m_pMemory;
    }

    iterator end() noexcept
    {
        return m_pMemory + m_nAllocationCount;
    }

    const_iterator end() const noexcept
    {
        return m_pMemory + m_nAllocationCount;
    }

    const_iterator cbegin() const noexcept
    {
        return m_pMemory;
    }

    const_iterator cend() const noexcept
    {
        return m_pMemory + m_nAllocationCount;
    }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    // element access
    T& operator[](I i)
    {
        AssertBool(IsIdxValid(i));

        return m_pMemory[i];
    }

    const T& operator[](I i) const
    {
        AssertBool(IsIdxValid(i));

        return m_pMemory[i];
    }

    T& Element(I i)
    {
        AssertBool(IsIdxValid(i));

        return m_pMemory[i];
    }

    const T& Element(I i) const
    {
        AssertBool(IsIdxValid(i));

        return m_pMemory[i];
    }

    bool IsIdxValid(I i) const
    {
        return (i >= 0) && (i < m_nAllocationCount);
    }

    
    static const I INVALID_INDEX = (I)-1; // For use with COMPILE_TIME_ASSERT
    static I       InvalidIndex() { return INVALID_INDEX; }

    T* Base()
    {
        return m_pMemory;
    }

    const T* Base() const
    {
        return m_pMemory;
    }

    // Size
    int NumAllocated() const
    {
        return m_nAllocationCount;
    }

    int Count() const
    {
        return m_nAllocationCount;
    }

    int Count()
    {
        return m_nAllocationCount;
    }

    void Grow(int num = 1)
    {
        if (num <= 0)
            return;

        if (IsReadOnly())
        {
            return;
        }

        if (static_cast<I>(num) > max_size() - m_nAllocationCount)
        {
            return;
        }

        int nAllocationRequested = m_nAllocationCount + num;

        int            nNewAllocationCount = UtlVectorMemory_CalcNewAllocationCount(m_nAllocationCount, m_nGrowSize & ~(EXTERNAL_CONST_BUFFER_MARKER | EXTERNAL_BUFFER_MARKER), nAllocationRequested, sizeof(T));
        constexpr auto max_possible_count  = std::numeric_limits<I>::max();

        if (static_cast<decltype(max_possible_count)>(nAllocationRequested) > max_possible_count)
        {
            return;
        }

        if (static_cast<decltype(max_possible_count)>(nNewAllocationCount) > max_possible_count)
        {
            nNewAllocationCount = max_possible_count;
        }

        nNewAllocationCount = std::min(nNewAllocationCount, static_cast<int>(max_possible_count));

        m_pMemory = static_cast<T*>(UtlVectorMemory_Alloc(m_pMemory, !IsExternallyAllocated(), nNewAllocationCount * sizeof(T), m_nAllocationCount * sizeof(T)));

        if (IsExternallyAllocated())
            m_nGrowSize &= ~(EXTERNAL_CONST_BUFFER_MARKER | EXTERNAL_BUFFER_MARKER);

        m_nAllocationCount = nNewAllocationCount;
    }

    // Makes sure we've got at least this much memory
    void EnsureCapacity(int num)
    {
        if (m_nAllocationCount >= num)
            return;

        if (IsReadOnly())
        {
            // Can't grow a buffer whose memory was externally allocated
            return;
        }

        m_pMemory = static_cast<T*>(UtlVectorMemory_Alloc(m_pMemory, !IsExternallyAllocated(), num * sizeof(T), m_nAllocationCount * sizeof(T)));

        if (IsExternallyAllocated())
            m_nGrowSize &= ~(EXTERNAL_CONST_BUFFER_MARKER | EXTERNAL_BUFFER_MARKER);

        m_nAllocationCount = num;
    }

    // Memory deallocation
    void Purge()
    {
        if (!IsExternallyAllocated())
        {
            if (m_pMemory)
            {
                FreeMemory(static_cast<void*>(m_pMemory));
                m_pMemory = nullptr;
            }
            m_nAllocationCount = 0;
        }
    }

    // Purge all but the given number of elements
    void Purge(int numElements)
    {
        if (numElements < 0)
            return;

        if (numElements > m_nAllocationCount)
        {
            return;
        }

        if (numElements == 0)
        {
            Purge();
            return;
        }

        if (IsReadOnly())
        {
            return;
        }

        if (numElements == m_nAllocationCount)
        {
            return;
        }

        if (!m_pMemory)
        {
            return;
        }

        m_pMemory = static_cast<T*>(UtlVectorMemory_Alloc(m_pMemory, !IsExternallyAllocated(), numElements * sizeof(T), m_nAllocationCount * sizeof(T)));

        if (IsExternallyAllocated())
            m_nGrowSize &= ~(EXTERNAL_CONST_BUFFER_MARKER | EXTERNAL_BUFFER_MARKER);

        m_nAllocationCount = numElements;
    }

    // is the memory externally allocated?
    bool IsExternallyAllocated() const
    {
        return (m_nGrowSize & (EXTERNAL_CONST_BUFFER_MARKER | EXTERNAL_BUFFER_MARKER)) != 0;
    }

    // is the memory read only?
    bool IsReadOnly() const
    {
        return (m_nGrowSize & EXTERNAL_CONST_BUFFER_MARKER) != 0;
    }

protected:
    using UnsignedI = std::make_unsigned_t<I>;

    // Use constexpr for compile-time constants
    static constexpr UnsignedI EXTERNAL_CONST_BUFFER_MARKER = UnsignedI(1) << (sizeof(I) * 8 - 2);
    static constexpr UnsignedI EXTERNAL_BUFFER_MARKER       = UnsignedI(1) << (sizeof(I) * 8 - 1);
    static constexpr UnsignedI FLAGS_MASK                   = EXTERNAL_CONST_BUFFER_MARKER | EXTERNAL_BUFFER_MARKER;
    static constexpr UnsignedI GROW_SIZE_MASK               = ~FLAGS_MASK;

    T* m_pMemory;
    I  m_nAllocationCount;
    I  m_nGrowSize;

private:
    constexpr I max_size() const
    {
        return std::numeric_limits<I>::max();
    }
};

template <class T, size_t SIZE, class I = int>
class CUtlMemoryFixedGrowable : public CUtlMemory<T, I>
{
    using BaseClass = CUtlMemory<T, I>;

public:
    CUtlMemoryFixedGrowable(int nGrowSize = 0, int nInitSize = SIZE) :
        BaseClass(m_pFixedMemory, SIZE)
    {
    }

private:
    T m_pFixedMemory[SIZE];
};

#endif
