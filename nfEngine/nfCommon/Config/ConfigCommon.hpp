/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config common types declaration.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

#define INVALID_NODE_PTR static_cast<uint32>(-1)

/**
 * Pointers to config structure elements (values, array nodes and object nodes).
 * We access them via index instead of pointers, because:
 * 1. They take less space on 64-bits.
 * 2. Buffers containing these elements will be dynamically expanded during parsing and inserting,
 *    so the pointers can turn invalid in the middle of these processes.
 */
typedef uint32 ConfigObjectNodePtr;
typedef uint32 ConfigArrayNodePtr;
typedef uint32 ConfigValuePtr;


class ConfigObject
{
    friend class Config;
    friend class ConfigValue;

    ConfigObjectNodePtr mHead;
    ConfigObjectNodePtr mTail;

public:
    ConfigObject() : mHead(INVALID_NODE_PTR), mTail(INVALID_NODE_PTR) { }
};

class ConfigArray
{
    friend class Config;
    friend class ConfigValue;

    ConfigArrayNodePtr mHead;
    ConfigArrayNodePtr mTail;

public:
    ConfigArray() : mHead(INVALID_NODE_PTR), mTail(INVALID_NODE_PTR) { }
};

/**
 * Config array node. Used for fast insertions and iterations.
 */
struct ConfigArrayNode
{
    ConfigValuePtr valuePtr;
    ConfigArrayNodePtr next;

    ConfigArrayNode() : valuePtr(INVALID_NODE_PTR), next(INVALID_NODE_PTR) { }
};

/**
 * Config object node. Used for fast insertions and iterations.
 */
struct ConfigObjectNode
{
    const char* name;
    ConfigValuePtr valuePtr;
    ConfigObjectNodePtr next;

    ConfigObjectNode() : name(nullptr), valuePtr(INVALID_NODE_PTR), next(INVALID_NODE_PTR) { }
};


} // namespace Common
} // namespace NFE
