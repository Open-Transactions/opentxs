/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_PROTO_HPP
#define OPENTXS_CORE_PROTO_HPP

// IWYU pragma: begin_exports
#include <opentxs-proto/Types.hpp>
#include <opentxs-proto/Check.hpp>
// IWYU pragma: end_exports

#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"

#include <iostream>

namespace opentxs
{
namespace proto
{
template<class T>
Data ProtoAsData(const T& serialized)
{
    auto size = serialized.ByteSize();
    char* protoArray = new char [size];

    OT_ASSERT_MSG(nullptr != protoArray, "protoArray failed to dynamically allocate.");

    serialized.SerializeToArray(protoArray, size);
    Data serializedData(protoArray, size);
    delete[] protoArray;
    return serializedData;
}

template<class T>
std::string ProtoAsString(const T& serialized)
{
    auto size = serialized.ByteSize();
    char* protoArray = new char [size];

    OT_ASSERT_MSG(nullptr != protoArray, "protoArray failed to dynamically allocate.");

    serialized.SerializeToArray(protoArray, size);
    std::string serializedData(protoArray, size);
    delete[] protoArray;
    return serializedData;
}

template<class T>
String ProtoAsArmored(const T& serialized, const String& header)
{
    auto data = ProtoAsData<T>(serialized);
    OTASCIIArmor armored(data);
    String output;
    armored.WriteArmoredString(output, header.Get());

    return output;
}

template<class T>
T RawToProto(const char* input, const size_t size)
{
    T serialized;
    serialized.ParseFromArray(input, size);

    return serialized;
}

template<class T>
T TextToProto(const std::string& input)
{
    return RawToProto<T>(input.data(), input.size());
}

template<class T>
T DataToProto(const Data& input)
{
    return RawToProto<T>(
        static_cast<const char*>(input.GetPointer()),
        input.GetSize());
}

template<class T>
T StringToProto(const String& input)
{
    OTASCIIArmor armored;
    String unconstInput(input);

    if (!armored.LoadFromString(unconstInput)) {
        std::cerr << __FUNCTION__ << "Failed to decode armored protobuf."
                  << std::endl;

        return T();
    } else {

        return DataToProto<T>(Data(armored));
    }
}

} // namespace proto
} // namespace opentxs

#endif // OPENTXS_CORE_PROTO_HPP
