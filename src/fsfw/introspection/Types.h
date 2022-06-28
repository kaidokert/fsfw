#pragma once

//maybe call them MIB types as these are the ones exposed to the MIB?
// Note: some DBs (Postgress, Mongo) only support signed 64bit integers. To have a common denominator, all integers are int64_t.
// As such, ther is no unsigned Type, as there can not be a uint64_t and uint32_t completely fits into int64_t
namespace Types {
enum ParameterType { SIGNED, FLOATING, ENUM, UNSUPPORTED };
}  // namespace Types