/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  BitMask class implementation.
 */

#pragma once

#include "PCH.hpp"
#include "BitMask.hpp"
#include "../System/Assertion.hpp"
#include "../Utils/BitUtils.hpp"
#include "../Logger/Logger.hpp"
#include "../Math/Math.hpp"


namespace NFE {
namespace Common {


BitMask::BitMask(const BitMask& other)
    : mSize(0)
{
    if (other.mSize != 0)
    {
        const size_t wordsRequired = other.GetNumOfWords();
        UniquePtr<size_t[]> newWords(new size_t[wordsRequired]);
        if (!newWords)
        {
            LOG_ERROR("Failed to allocate memory for bit mask");
            return;
        }

        memcpy(newWords.Get(), mWords.Get(), wordsRequired * sizeof(size_t));
        mWords = std::move(newWords);
        mSize = other.mSize;
    }
}

BitMask::BitMask(BitMask&& other)
    : mWords(std::move(other.mWords))
    , mSize(other.mSize)
{
    other.mSize = 0;
}

BitMask& BitMask::operator = (const BitMask& other)
{
    if (other.mSize == 0)
    {
        Resize(0);
        return *this;
    }

    const size_t wordsRequired = other.GetNumOfWords();
    UniquePtr<size_t[]> newWords(new size_t[wordsRequired]);
    if (!newWords)
    {
        LOG_ERROR("Failed to allocate memory for bit mask");
        return *this;
    }

    memcpy(newWords.Get(), mWords.Get(), wordsRequired * sizeof(size_t));
    mWords = std::move(newWords);
    mSize = other.mSize;

    return *this;
}

BitMask& BitMask::operator = (BitMask&& other)
{
    mWords = std::move(other.mWords);
    mSize = other.mSize;
    other.mSize = 0;
    return *this;
}

bool BitMask::Resize(size_t size, bool preserveValues)
{
    if (size == 0)
    {
        mWords.Reset();
        mSize = 0;
        return true;
    }

    const size_t wordsRequired = (size + (8 * sizeof(size_t) - 1)) / sizeof(size_t);
    UniquePtr<size_t[]> newWords(new size_t[wordsRequired]);
    if (!newWords)
    {
        LOG_ERROR("Failed to allocate memory for bit mask");
        return false;
    }

    if (preserveValues)
    {
        const size_t numCommonWords = Math::Min(GetNumOfWords(), wordsRequired);
        memcpy(newWords.Get(), mWords.Get(), numCommonWords * sizeof(size_t));
    }

    mWords = std::move(newWords);
    mSize = size;
    return true;
}

bool BitMask::operator[](size_t index) const
{
    NFE_ASSERT(index < mSize, "Invalid bit mask index");

    const size_t wordIndex = index / (8 * sizeof(size_t));
    const size_t bitIndex = index % (8 * sizeof(size_t));
    return (mWords[wordIndex] >> bitIndex) != 0;
}

bool BitMask::All() const
{
    const size_t numWords = GetNumOfWords();
    for (size_t i = 0; i < numWords - 1; ++i)
    {
        if (mWords[i] != ~static_cast<size_t>(0))
        {
            return false;
        }
    }

    // check last word
    return BitUtils<size_t>::CountTrailingZeros(~mWords[numWords - 1]) >= GetNumBitsInLastWord();
}

bool BitMask::Any() const
{
    const size_t numWords = GetNumOfWords();
    for (size_t i = 0; i < numWords - 1; ++i)
    {
        if (mWords[i] != 0)
        {
            return true;
        }
    }

    // check last word
    return BitUtils<size_t>::CountTrailingZeros(mWords[numWords - 1]) < GetNumBitsInLastWord();
}

bool BitMask::None() const
{
    const size_t numWords = GetNumOfWords();
    for (size_t i = 0; i < numWords - 1; ++i)
    {
        if (mWords[i] != 0)
        {
            return false;
        }
    }

    // check last word
    return BitUtils<size_t>::CountTrailingZeros(mWords[numWords - 1]) >= GetNumBitsInLastWord();
}

size_t BitMask::Count() const
{
    const size_t numWords = GetNumOfWords();
    size_t result = BitUtils<size_t>::CountBits(mWords[numWords - 1] & GetLastWordMask());

    for (size_t i = 0; i < numWords - 1; ++i)
    {
        result += BitUtils<size_t>::CountBits(mWords[i]);
    }

    return result;
}

BitMask& BitMask::Set(size_t index, bool value)
{
    NFE_ASSERT(index < mSize, "Invalid bit mask index");
    const size_t wordIndex = index / (8 * sizeof(size_t));
    const size_t bitIndex = index % (8 * sizeof(size_t));
    mWords[wordIndex] = mWords[wordIndex] & ~(static_cast<size_t>(1) << bitIndex) | (static_cast<size_t>(value) << bitIndex);
    return *this;
}

BitMask& BitMask::Set(const BitMask& mask, bool value)
{
    // TODO
    return *this;
}

BitMask& BitMask::Set(bool value)
{
    size_t mask = 0;
    if (value)
        mask = ~mask;

    const size_t numWords = GetNumOfWords();
    for (size_t i = 0; i < numWords; ++i)
    {
        mWords[i] = mask;
    }

    return *this;
}

BitMask& BitMask::Toggle(size_t index)
{
    NFE_ASSERT(index < mSize, "Invalid bit mask index");
    const size_t wordIndex = index / (8 * sizeof(size_t));
    const size_t bitIndex = index % (8 * sizeof(size_t));
    mWords[wordIndex] ^= static_cast<size_t>(1) << bitIndex;
    return *this;
}

BitMask& BitMask::Toggle()
{
    const size_t numWords = GetNumOfWords();
    for (size_t i = 0; i < numWords; ++i)
    {
        mWords[i] = ~mWords[i];
    }

    return *this;
}

BitMask& BitMask::operator &= (const BitMask& other)
{
    // TODO
    return *this;
}

BitMask& BitMask::operator |= (const BitMask& other)
{
    // TODO
    return *this;
}

BitMask& BitMask::operator ^= (const BitMask& other)
{
    // TODO
    return *this;
}

BitMask BitMask::operator~() const
{
    BitMask result(*this);
    result.Toggle();
    return result;
}

bool BitMask::operator == (const BitMask& other)
{
    return false;
}

bool BitMask::operator != (const BitMask& other)
{
    return false;
}

} // namespace Common
} // namespace NFE
