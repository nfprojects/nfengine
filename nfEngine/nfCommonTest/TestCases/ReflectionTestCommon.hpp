#pragma once

#include "nfCommon/nfCommon.hpp"

#include <string>


namespace helper {

// serialize object to string
void SerializeObject(const NFE::RTTI::Type* type, const void* object, std::string& outString);

// deserialize object from string
void DeserializeObject(const NFE::RTTI::Type* type, void* outObject, const std::string& string);

} // namespace helper
