#pragma once

struct ClassMethodCallCounters
{
    int constructor;
    int copyConstructor;
    int assignment;
    int moveConstructor;
    int moveAssignment;
    int destructor;

    ClassMethodCallCounters()
        : constructor(0)
        , copyConstructor(0)
        , assignment(0)
        , moveConstructor(0)
        , moveAssignment(0)
        , destructor(0)
    {}
};


/**
 * a helper class that can be only copied
 */
template<typename T>
class CopyOnlyTestClass
{
public:
    CopyOnlyTestClass(ClassMethodCallCounters* counters, const T& payload = T())
        : mCounters(counters)
        , mPayload(payload)
    {
        mCounters->constructor++;
    }

    CopyOnlyTestClass(const CopyOnlyTestClass& rhs)
        : mCounters(rhs.mCounters)
    {
        mCounters->copyConstructor++;
    }

    CopyOnlyTestClass& operator=(const CopyOnlyTestClass& rhs)
    {
        mCounters = rhs.mCounters;
        mCounters->assignment++;
        return *this;
    }

    ~CopyOnlyTestClass()
    {
        mCounters->destructor++;
    }

private:
    CopyOnlyTestClass(CopyOnlyTestClass&& rhs) = delete;
    CopyOnlyTestClass& operator=(CopyOnlyTestClass&& rhs) = delete;

    ClassMethodCallCounters* mCounters;
    T mPayload;
};


/**
 * A helper class that can be only moved
 */
template<typename T>
class MoveOnlyTestClass
{
public:
    MoveOnlyTestClass(ClassMethodCallCounters* counters, const T& payload = T())
        : mCounters(counters)
        , mPayload(payload)
    {
        mCounters->constructor++;
    }

    MoveOnlyTestClass(MoveOnlyTestClass&& rhs)
        : mCounters(rhs.mCounters)
    {
        mCounters->moveConstructor++;
        rhs.mCounters = nullptr;
    }

    MoveOnlyTestClass& operator=(MoveOnlyTestClass&& rhs)
    {
        mCounters = rhs.mCounters;
        rhs.mCounters = nullptr;
        mCounters->moveAssignment++;
        return *this;
    }

    ~MoveOnlyTestClass()
    {
        if (mCounters)
        {
            mCounters->destructor++;
        }
    }

private:
    MoveOnlyTestClass(const MoveOnlyTestClass& rhs) = delete;
    MoveOnlyTestClass& operator=(const MoveOnlyTestClass& rhs) = delete;

    ClassMethodCallCounters* mCounters;
    T mPayload;
};