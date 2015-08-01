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
 * functionality as std::map<IDType, ObjType> (where IDType is unsigned integer type),
 * but all operations have O(1) average complexity.
 *
 * @p ObjType describes objects types hold in the array.
 * @p IDType describes index type used to access the objects. It allows to choose an arbitrary
 * unsigned index size (unsigned char, unsigned short, unsigned int, etc.) in order to reduce
 * indices list memory footprint and gain additional performance (at the expense of lower
 * capacity).
 */
template<typename ObjType,
    typename IDType = unsigned int>
class PackedArray
{
    static_assert(std::numeric_limits<IDType>::is_integer, "IDType must be integer type");
    static_assert(!std::numeric_limits<IDType>::is_signed, "IDType must be unsigned type");

private:
    // doubly link list node
    struct ListNode
    {
        IDType next;
        IDType prev;
    };

    size_t mSize;        //< total allocated space (in objects)
    size_t mUsed;        //< created objects
    IDType* mIDs;        //< array of object IDs
    ListNode* mNodes;    //< list nodes for free and taken objects
    ObjType* mObjects;   //< block of allocated objects
    IDType mFreeHead;    //< head of free objects list
    IDType mTakenHead;   //< head of taken objects list

    /// we don't want these
    PackedArray(const PackedArray&) = delete;
    PackedArray(PackedArray&&) = delete;
    PackedArray& operator=(const PackedArray&) = delete;
    PackedArray& operator=(PackedArray&&) = delete;

    bool Resize(size_t newSize)
    {
        assert(newSize >= 4);
        assert(newSize <= MaxSize());

        // TODO: support aligned allocation
        ObjType* newObjects = static_cast<ObjType*>(realloc(mObjects, sizeof(ObjType) * newSize));
        ListNode* newNodes = static_cast<ListNode*>(realloc(mNodes, sizeof(ListNode) * newSize));
        IDType* newIDs = static_cast<IDType*>(realloc(mIDs, sizeof(IDType) * newSize));

        // allocation failed
        if (newNodes == nullptr || newObjects == nullptr || newIDs == nullptr)
            return false;

        mNodes = newNodes;
        mObjects = newObjects;
        mIDs = newIDs;

        // build list of free object slots

        /// first index
        mIDs[mSize] = InvalidIndex;
        mNodes[mSize].next = static_cast<IDType>(mSize + 1);
        mNodes[mSize].prev = InvalidIndex;

        /// "middle"
        for (size_t i = mSize + 1; i < newSize - 1; ++i)
        {
            mIDs[i] = InvalidIndex;
            mNodes[i].next = static_cast<IDType>(i + 1);
            mNodes[i].prev = static_cast<IDType>(i - 1);
        }

        /// last index
        mIDs[newSize - 1] = InvalidIndex;
        mNodes[newSize - 1].next = static_cast<IDType>(mFreeHead);
        mNodes[newSize - 1].prev = static_cast<IDType>(newSize - 2);

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
        mIDs = nullptr;
        mNodes = nullptr;
        mObjects = nullptr;
        mFreeHead = InvalidIndex;
        mTakenHead = InvalidIndex;

        Resize(initialSize);
    }

    ~PackedArray()
    {
        if (mNodes)
            free(mNodes);
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
        mFreeHead = mNodes[index].next;
        if (mFreeHead != InvalidIndex)
            mNodes[mFreeHead].prev = InvalidIndex;

        if (mTakenHead != InvalidIndex)
            mNodes[mTakenHead].prev = index;

        mIDs[index] = id;
        mNodes[index].next = mTakenHead;
        mNodes[index].prev = InvalidIndex;
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
            (index < MaxSize()) &&  // check if index is in range of the max array size
            (mIDs[index] < mUsed);  // check if index points to object in range of objects table
    }

    /**
     * Remove an object from the array.
     * @param index Index of an object to be removed
     */
    void Remove(IDType index)
    {
        assert(Has(index));

        IDType id = mIDs[index];
        --mUsed;
        if (id < mUsed)
        {
            // move the object from the end to fill the gap
            mObjects[id] = mObjects[mUsed];
        }

        IDType newLastTaken = mNodes[mTakenHead].next;

        if (newLastTaken == index)
            newLastTaken = mTakenHead;

        if (mNodes[mTakenHead].next != InvalidIndex)
            mNodes[mNodes[mTakenHead].next].prev = InvalidIndex;

        /// update indicies of moved object
        mIDs[mTakenHead] = mIDs[index];
        mNodes[mTakenHead].next = mNodes[index].next;
        mNodes[mTakenHead].prev = mNodes[index].prev;

        /// correct neighbours pointers
        if (mNodes[mTakenHead].prev != InvalidIndex)
            mNodes[mNodes[mTakenHead].prev].next = mTakenHead;
        if (mNodes[mTakenHead].next != InvalidIndex)
            mNodes[mNodes[mTakenHead].next].prev = mTakenHead;

        mTakenHead = newLastTaken;

        // update free indices list
        mIDs[index] = InvalidIndex;
        mNodes[index].prev = InvalidIndex;
        mNodes[index].next = mFreeHead;
        mFreeHead = index;

        if (mTakenHead != InvalidIndex)
            assert(mIDs[mTakenHead] == (mUsed - 1));
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
        return mObjects[mIDs[index]];
    }

    /**
     * Object access.
     * @param index Object index.
     * @return Constant object reference.
     */
    const ObjType& operator[](IDType index) const
    {
        assert(Has(index));
        return mObjects[mIDs[index]];
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
                << "id = " << mIDs[i]
                << ", next = " << mNodes[i].next
                << ", prev = " << mNodes[i].prev << std::endl;
        }
    }
};

} // namespace Common
} // namespace NFE