/**
 * @file
 * @author Michal Witanowski <witek902@gmail.com>
 * @brief  PackedArray template class definition
 */

#include <limits>
#include <algorithm>
#include <functional>
#include <assert.h>

namespace NFE {
namespace Common {

/**
 * PackedArray is a class that holds all objects in a contiguous memory block, even if they are
 * created and removed in a random order. PackedArray<ObjType, IDType> provides simillar
 * functionality as std::map<IDType, ObjType>, but all operations have O(1) average complexity.
 *
 * @p ObjType describes objects types hold in the array.
 * @p IDType describes index type used to access the objects. It allows to choose an arbitrary
 * index size (char, short, int, etc.) in order to reduce array capacity and gain additional
 * performance.
 */
template<typename ObjType,
    typename IDType = unsigned int>
class PackedArray
{
    static_assert(std::numeric_limits<IDType>::is_integer, "IDType must be integer type");
    static_assert(!std::numeric_limits<IDType>::is_signed, "IDType must be unsigned type");

private:
    struct Index
    {
        IDType id;
        IDType next;
        IDType prev;
    };

    size_t mSize;
    size_t mUsed;
    Index* mIndices;
    ObjType* mObjects;
    IDType mFreeHead;
    IDType mTakenHead;

    /// we don't want these
    PackedArray(const PackedArray&) = delete;
    PackedArray(PackedArray&&) = delete;
    PackedArray& operator=(const PackedArray&) = delete;
    PackedArray& operator=(PackedArray&&) = delete;

    bool Resize(size_t newSize)
    {
        assert(newSize >= 4);
        assert(newSize <= MaxSize());

        // TODO: aligned allocation
        ObjType* newObjects = static_cast<ObjType*>(realloc(mObjects, sizeof(ObjType) * newSize));
        Index* newIndicies = static_cast<Index*>(realloc(mIndices, sizeof(Index) * newSize));

        // allocation failed
        if (newIndicies == nullptr || newObjects == nullptr)
            return false;

        mIndices = newIndicies;
        mObjects = newObjects;

        // build free-list:

        /// first index
        mIndices[mSize].id = InvalidIndex;
        mIndices[mSize].next = static_cast<IDType>(mSize + 1);
        mIndices[mSize].prev = InvalidIndex;

        /// "middle"
        for (size_t i = mSize + 1; i < newSize - 1; ++i)
        {
            mIndices[i].id = InvalidIndex;
            mIndices[i].next = static_cast<IDType>(i + 1);
            mIndices[i].prev = static_cast<IDType>(i - 1);
        }

        /// last index
        mIndices[newSize - 1].id = InvalidIndex;
        mIndices[newSize - 1].next = static_cast<IDType>(mFreeHead);
        mIndices[newSize - 1].prev = static_cast<IDType>(newSize - 2);

        /// update pointers
        if (mFreeHead == InvalidIndex)
            mFreeHead = static_cast<IDType>(mSize);

        mSize = newSize;
        return true;
    }

public:
    const IDType InvalidIndex = static_cast<IDType>(-1);

    PackedArray(size_t initialSize = 64)
    {
        mSize = 0;
        mUsed = 0;
        mIndices = nullptr;
        mObjects = nullptr;
        mFreeHead = InvalidIndex;
        mTakenHead = InvalidIndex;

        Resize(initialSize);
    }

    ~PackedArray()
    {
        if (mIndices)
            free(mIndices);
        if (mObjects)
            free(mObjects);
    }

    /**
     * Add an object to the array.
     * @param obj Object to be added
     * @return    Object index
     */
    IDType Add(const ObjType& obj)
    {
        if (mUsed == MaxSize())
            return InvalidIndex;

        if (mFreeHead == InvalidIndex)
        {
            size_t newSize = std::min(MaxSize(), mSize * 2);
            if (!Resize(newSize))
                return InvalidIndex;
        }

        // get next free index and update the free list
        IDType index = mFreeHead;
        IDType id = static_cast<IDType>(mUsed); // next free object is always at the end

        // update free indices list
        mFreeHead = mIndices[index].next;
        if (mFreeHead != InvalidIndex)
            mIndices[mFreeHead].prev = InvalidIndex;

        if (mTakenHead != InvalidIndex)
            mIndices[mTakenHead].prev = index;

        mIndices[index].id = id;
        mIndices[index].next = mTakenHead;
        mIndices[index].prev = InvalidIndex;
        mTakenHead = index;

        *(mObjects + id) = obj;
        mUsed++;

        return index;
    }

    /**
     * Check if provided index is a valid index.
     */
    bool Has(IDType index)
    {
        return
            (index >= 0) && (index < mSize) &&  // check if index is in range of index table
            (index < MaxSize()) &&              // check if index is in range of the max array size
            (mIndices[index].id < mUsed);       // check if index points to object in range
        // of objects table
    }

    /**
     * Remove an object from the array.
     * @param index Index of an object to be removed
     */
    void Remove(IDType index)
    {
        assert(Has(index));

        IDType id = mIndices[index].id;
        --mUsed;
        if (id < mUsed)
        {
            // move the object from the end to fill the gap
            mObjects[id] = mObjects[mUsed];
        }

        IDType newLastTaken = mIndices[mTakenHead].next;

        if (newLastTaken == index)
            newLastTaken = mTakenHead;

        if (mIndices[mTakenHead].next != InvalidIndex)
            mIndices[mIndices[mTakenHead].next].prev = InvalidIndex;

        /// update indicies of moved object
        mIndices[mTakenHead].id = mIndices[index].id;
        mIndices[mTakenHead].next = mIndices[index].next;
        mIndices[mTakenHead].prev = mIndices[index].prev;

        /// correct neighbours pointers
        if (mIndices[mTakenHead].prev != InvalidIndex)
            mIndices[mIndices[mTakenHead].prev].next = mTakenHead;
        if (mIndices[mTakenHead].next != InvalidIndex)
            mIndices[mIndices[mTakenHead].next].prev = mTakenHead;

        mTakenHead = newLastTaken;

        // update free indices list
        mIndices[index].id = InvalidIndex;
        mIndices[index].prev = InvalidIndex;
        mIndices[index].next = mFreeHead;
        mFreeHead = index;

        if (mTakenHead != InvalidIndex)
            assert(mIndices[mTakenHead].id == (mUsed - 1));
        assert(mTakenHead != mFreeHead);
    }

    /**
     * Object access.
     * @param index Object index.
     * @return Object reference.
     */
    ObjType& operator[](IDType index)
    {
        assert(Has(index));
        return mObjects[mIndices[index].id];
    }

    /**
     * Object access.
     * @param index Object index.
     * @return Constant object reference.
     */
    const ObjType& operator[](IDType index) const
    {
        assert(Has(index));
        return mObjects[mIndices[index].id];
    }

    /**
     * Iterate through all the objects the fastest possible way.
     * @param func Function to be called for each object.
     */
    void Iterate(std::function<void(ObjType&)> func) const
    {
        for (size_t i = 0; i < mUsed; ++i)
            func(mObjects[i]);
    }

    /**
     * Get number of objects in the array.
     */
    size_t Size() const
    {
        return mUsed;
    }

    /**
     * Get maximum array size (in objects count) with given @p IDType.
     */
    static size_t MaxSize()
    {
        return static_cast<IDType>(-1);
    }

    void DebugPrint()
    {
        std::cout << "Usage: " << mUsed << "/" << mSize << std::endl;
        std::cout << "NextFree: " << mFreeHead << std::endl;
        std::cout << "LastTaken: " << mTakenHead << std::endl;

        for (size_t i = 0; i < mSize; ++i)
        {
            std::cout << i << ": "
                << "id = " << mIndices[i].id
                << ", next = " << mIndices[i].next
                << ", prev = " << mIndices[i].prev << std::endl;
        }
    }
};

} // namespace Common
} // namespace NFE