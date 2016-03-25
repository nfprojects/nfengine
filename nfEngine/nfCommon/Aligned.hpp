/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of memory aligning utilities.
 */

#pragma once

#include "nfCommon.hpp"

namespace NFE {
namespace Common {

// workaround for Visual Studio compiler bug that generates
// "warning C4100: 'p': unreferenced formal parameter" in AlignedAllocator::destroy
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100)
#endif // _MSC_VER


/**
 * Override this class to align children objects.
 */
template <size_t Alignment = 16>
class Aligned
{
public:
    virtual ~Aligned() { }

#if defined(WIN32)
    void* operator new(size_t size)
    {
        return _aligned_malloc(size, Alignment);
    }

    void operator delete(void* ptr)
    {
        _aligned_free(ptr);
    }
#elif defined(__LINUX__) | defined(__linux__)
    void* operator new(size_t size)
    {
        return aligned_alloc(Alignment, size);
    }

    void operator delete(void* ptr)
    {
        free(ptr);
    }
#else
#error "Target system not supported!"
#endif // defined(WIN32)

    // placement new
    void* operator new(size_t size, void* ptr)
    {
        (void)size;
        return ptr;
    }
};


/**
 * This code comes from https://gist.github.com/donny-dont/1471329
 *
 * TODO rewrite the implementation
 */
template <typename T, size_t Alignment>
class AlignedAllocator
{
public:
    // The following will be the same for virtually all allocators.
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef std::ptrdiff_t difference_type;

    T* address(T& r) const
    {
        return &r;
    }

    const T* address(const T& s) const
    {
        return &s;
    }

    size_t max_size() const
    {
        // The following has been carefully written to be independent of
        // the definition of size_t and to avoid signed/unsigned warnings.
        return (static_cast<size_t>(0) - static_cast<size_t>(1)) / sizeof(T);
    }

    // The following must be the same for all allocators.
    template <typename U>
    struct rebind
    {
        typedef AlignedAllocator<U, Alignment> other;
    } ;

    bool operator!=(const AlignedAllocator& other) const
    {
        return !(*this == other);
    }

    void construct(T* const p, const T& t) const
    {
        void* const pv = static_cast<void*>(p);
        new (pv) T(t);
    }

    void destroy(T* const p) const
    {
        p->~T();
    }

    // Returns true if and only if storage allocated from *this
    // can be deallocated from other, and vice versa.
    // Always returns true for stateless allocators.
    bool operator==(const AlignedAllocator&) const
    {
        return true;
    }

    // Default constructor, copy constructor, rebinding constructor, and destructor.
    // Empty for stateless allocators.
    AlignedAllocator() { }
    AlignedAllocator(const AlignedAllocator&) { }
    template <typename U> AlignedAllocator(const AlignedAllocator<U, Alignment>&) { }
    ~AlignedAllocator() { }

    // The following will be different for each allocator.
    T* allocate(const size_t n) const
    {
        // The return value of allocate(0) is unspecified.
        // Mallocator returns NULL in order to avoid depending
        // on malloc(0)'s implementation-defined behavior
        // (the implementation can define malloc(0) to return NULL,
        // in which case the bad_alloc check below would fire).
        // All allocators can return NULL in this case.
        if (n == 0) return nullptr;

        // All allocators should contain an integer overflow check.
        // The Standardization Committee recommends that std::length_error
        // be thrown in the case of integer overflow.
        if (n > max_size())
            throw std::length_error("AlignedAllocator<T>::allocate() - Integer overflow.");

        // Mallocator wraps malloc().
#if defined(WIN32)
        void* const pv = _aligned_malloc(n * sizeof(T), Alignment);
#elif defined(__LINUX__) | defined(__linux__)
        void* const pv = aligned_alloc(Alignment, n * sizeof(T));
#endif
        // Allocators should throw std::bad_alloc in the case of memory allocation failure.
        if (nullptr == pv)
            throw std::bad_alloc();

        return static_cast<T*>(pv);
    }

    void deallocate(T* const p, const size_t) const
    {
#if defined(WIN32)
        _aligned_free(p);
#elif defined(__LINUX__) | defined(__linux__)
        free(p);
#endif
    }

    // The following will be the same for all allocators that ignore hints.
    template <typename U>
    T* allocate(const size_t n, const U* /* const hint */) const
    {
        return allocate(n);
    }

    // Allocators are not required to be assignable, so
    // all allocators should have a private unimplemented
    // assignment operator. Note that this will trigger the
    // off-by-default (enabled under /Wall) warning C4626
    // "assignment operator could not be generated because a
    // base class assignment operator is inaccessible" within
    // the STL headers, but that warning is useless.
private:
    AlignedAllocator& operator=(const AlignedAllocator&);
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

} // namespace Common
} // namespace NFE
