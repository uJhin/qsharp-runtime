// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <cassert>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <cstdint>

#include "QirTypes.hpp"
#include "QirRuntime.hpp"

// Exposed to tests only:
std::unordered_map<std::string, QirString*>& AllocatedStrings();


std::unordered_map<std::string, QirString*>& AllocatedStrings() // Cannot be static, is called by tests.
{
    static std::unordered_map<std::string, QirString*> allocatedStrings;
    return allocatedStrings;
}

static QirString* CreateOrReuseAlreadyAllocated(std::string&& str)
{
    QirString* qstr       = nullptr;
    auto alreadyAllocated = AllocatedStrings().find(str);
    if (alreadyAllocated != AllocatedStrings().end())
    {
        qstr = alreadyAllocated->second;
        assert(qstr->str == str);
        qstr->refCount++;
    }
    else
    {
        qstr                          = new QirString(std::move(str));
        AllocatedStrings()[qstr->str] = qstr;
    }
    return qstr;
}

QirString::QirString(std::string&& otherStr)
    // clang-format off
    : str(std::move(otherStr))
// clang-format on
{
}

QirString::QirString(const char* cstr)
    // clang-format off
    : str(cstr)
// clang-format on
{
}


extern "C"
{
    // Creates a string from an array of UTF-8 bytes.
    QirString* __quantum__rt__string_create(const char* bytes) // NOLINT
    {
        return CreateOrReuseAlreadyAllocated(std::string(bytes));
    }

    void __quantum__rt__string_update_reference_count(QirString* qstr, int32_t increment) // NOLINT
    {
        if (qstr == nullptr || increment == 0)
        {
            return;
        }

        assert(qstr->refCount > 0 && "The string has been already released!");
        qstr->refCount += increment;

        if (qstr->refCount < 0)
        {
            __quantum__rt__fail(__quantum__rt__string_create("Attempting to decrement reference count below zero!"));
        }
        else if (qstr->refCount == 0)
        {
            auto allocated = AllocatedStrings().find(qstr->str);
            assert(allocated != AllocatedStrings().end());
            // TODO: consider amortizing map cleanup across multiple iterations
            AllocatedStrings().erase(allocated);
            delete qstr;
        }
    }

    // Creates a new string that is the concatenation of the two argument strings.
    QirString* __quantum__rt__string_concatenate(QirString* left, QirString* right) // NOLINT
    {
        return CreateOrReuseAlreadyAllocated(left->str + right->str);
    }

    // Returns true if the two strings are equal, false otherwise.
    bool __quantum__rt__string_equal(QirString* left, QirString* right) // NOLINT
    {
        assert((left == right) == (left->str == right->str));
        return left == right;
    }

    // Returns a string representation of the integer.
    QirString* __quantum__rt__int_to_string(int64_t value) // NOLINT
    {
        return CreateOrReuseAlreadyAllocated(std::to_string(value));
    }

    // Returns a string representation of the double.
    QirString* __quantum__rt__double_to_string(double value) // NOLINT
    {
        std::ostringstream oss;
        oss.precision(std::numeric_limits<double>::max_digits10);
        oss << std::fixed << value;
        std::string str = oss.str();

        // Remove padding zeros from the decimal part (relies on the fact that the output for integers always contains
        // period).
        std::size_t pos1 = str.find_last_not_of('0');
        if (pos1 != std::string::npos)
        {
            str.erase(pos1 + 1);
        }
        // For readability don't end with "." -- always have at least one digit in the decimal part.
        if (str[str.size() - 1] == '.')
        {
            str.append("0");
        }

        return CreateOrReuseAlreadyAllocated(std::move(str));
    }

    // Returns a string representation of the Boolean.
    QirString* __quantum__rt__bool_to_string(bool value) // NOLINT
    {
        std::string str = value ? "true" : "false";
        return CreateOrReuseAlreadyAllocated(std::move(str));
    }

    // Returns a string representation of the Pauli.
    QirString* __quantum__rt__pauli_to_string(PauliId pauli) // NOLINT
    {
        switch (pauli)
        {
        case PauliId_I:
            return __quantum__rt__string_create("PauliI");
        case PauliId_X:
            return __quantum__rt__string_create("PauliX");
        case PauliId_Y:
            return __quantum__rt__string_create("PauliY");
        case PauliId_Z:
            return __quantum__rt__string_create("PauliZ");
        default:
            break;
        }
        return __quantum__rt__string_create("<Unexpected Pauli Value>");
    }

    // Returns a string representation of the range.
    QirString* quantum__rt__range_to_string(const QirRange& range) // NOLINT
    {
        std::ostringstream oss;
        oss << range.start << "..";
        if (range.step != 1)
        {
            oss << range.step << "..";
        }
        oss << range.end;

        return __quantum__rt__string_create(oss.str().c_str());
    }

    const char* __quantum__rt__string_get_data(QirString* str) // NOLINT
    {
        return str->str.c_str();
    }

    uint32_t __quantum__rt__string_get_length(QirString* str) // NOLINT
    {
        return (uint32_t)(str->str.size());
    }

    // Implemented in delegated.cpp:
    // QirString* __quantum__rt__qubit_to_string(QUBIT* qubit); // NOLINT

    // Returns a string representation of the big integer.
    // TODO QirString* __quantum__rt__bigint_to_string(QirBigInt*); // NOLINT
}
