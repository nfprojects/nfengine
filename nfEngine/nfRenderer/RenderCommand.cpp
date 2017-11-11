/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of RenderCommand class.
 */

#include "PCH.hpp"
#include "RenderCommand.hpp"

#include "../nfCommon/Memory/DefaultAllocator.hpp"

namespace NFE {
namespace Renderer {

//Merge sorting
//array - array of elements to be sorted
//n - number of elements in array
//temp - temporary array (for internal usage), its size must be >= n
//pComparator - comparing function
//pUserData - context
template <typename T>
void merge_sort(T* array, size_t n, T* temp, int (*pComparator)(const T&, const T&, void*),
                void* pUserData)
{
    size_t l1, l2, u1, u2, i;
    size_t size = 1, j, k;

    while (size < n)
    {
        l1 = 0;
        k = 0;
        while (l1 + size < n)
        {
            l2 = l1 + size;
            u1 = l2 - 1;
            u2 = (l2 + size - 1 < n) ? l2 + size - 1 : n - 1;

            for (i = l1, j = l2; i <= u1 && j <= u2; k++)
            {
                if (pComparator(array[i], array[j], pUserData) <= 0)
                    temp[k] = array[i++];
                else
                    temp[k] = array[j++];
            }

            for (; i <= u1; k++)
                temp[k] = array[i++];
            for (; j <= u2; k++)
                temp[k] = array[j++];

            l1 = u2 + 1;
        }

        for (i = l1; k < n; i++)
            temp[k++] = array[i];

        for (i = 0; i < n; i++)
            array[i] = temp[i];

        size *= 2;
    }
}

int RenderCommandDistanceCmp(const int& a, const int& b, void* p)
{
    const RenderCommand* pList = (RenderCommand*)p;
    const RenderCommand* pA = pList + a;
    const RenderCommand* pB = pList + b;

    float distA = pA->distance;
    float distB = pB->distance;

    if (distA > distB) return 1;
    if (distA < distB) return -1;
    return 0;
}

int RenderCommandMaterialCmp(const int& a, const int& b, void* p)
{
    const RenderCommand* pList = (RenderCommand*)p;
    const RenderCommand* pA = pList + a;
    const RenderCommand* pB = pList + b;

    size_t p_a = (size_t)(pA->material);
    size_t p_b = (size_t)(pB->material);

    if (p_a == p_b) return 0;

    if (p_a > p_b)
        return 1;
    return -1;
}

int RenderCommandMeshCmp(const int& a, const int& b, void* p)
{
    const RenderCommand* pList = (RenderCommand*)p;
    const RenderCommand* pA = pList + a;
    const RenderCommand* pB = pList + b;

    size_t p_a = (size_t)(pA->vertexBuffer.Get()); // TODO: IB too!
    size_t p_b = (size_t)(pB->vertexBuffer.Get());

    if (p_a == p_b)
    {
        return (int)pA->startIndex - (int)pB->startIndex;
    }

    if (p_a > p_b)
        return 1;
    return -1;
}

int RenderCommandCmp(const int& a, const int& b, void* p)
{
    const RenderCommand* pList = (RenderCommand*)p;
    const RenderCommand* pA = pList + a;
    const RenderCommand* pB = pList + b;

    //compare by material
    size_t p_a = (size_t)(pA->material);
    size_t p_b = (size_t)(pB->material);
    if (p_a > p_b)
        return 1;
    else if (p_a < p_b)
        return -1;

    //compare by material
    int p_c = 0;
    p_a = (size_t)(pA->vertexBuffer.Get()); // TODO: IB too!
    p_b = (size_t)(pB->vertexBuffer.Get());
    if (p_a > p_b)
        return 1;
    else if (p_a < p_b)
        return -1;
    p_c = (int)pA->startIndex - (int)pB->startIndex;
    if (p_c != 0)
        return p_c;


    //compare by distance
    float distA = pA->distance;
    float distB = pB->distance;
    if (distA > distB) return 1;
    if (distA < distB) return -1;
    return 0;
}

void RenderCommandBuffer::PushBack(const RenderCommand& command)
{
    commands.PushBack(command);
}

void RenderCommandBuffer::Clear()
{
    commands.Clear();
}

void RenderCommandBuffer::Sort()
{
    uint32 size = commands.Size();

    if (size <= 1)
        return;

    // build array of pointers and manipulate them instead of RenderCommand structures
    // TODO avoid per-frame allocations
    int* pPointers = reinterpret_cast<int*>(NFE_MALLOC(sizeof(int) * size, 1));
    for (size_t i = 0; i < size; i++)
        pPointers[i]  = static_cast<int>(i); // assume there is less than 2^32 items

    int* pTmpPointers = reinterpret_cast<int*>(NFE_MALLOC(sizeof(int) * size, 1));

    merge_sort<int>(pPointers, size, pTmpPointers, RenderCommandCmp, &commands[0]);

    NFE_FREE(pTmpPointers);

    //reorganize
    RenderCommand* pTmpCommands = reinterpret_cast<RenderCommand*>(
        NFE_MALLOC(sizeof(RenderCommand) * size, 16));
    memcpy(pTmpCommands, &commands[0], sizeof(RenderCommand) * size );
    for (size_t i = 0; i < commands.size(); i++)
    {
        commands[i] = pTmpCommands[pPointers[i]];
    }
    NFE_FREE(pTmpCommands);

    NFE_FREE(pPointers);
}

} // namespace Renderer
} // namespace NFE
