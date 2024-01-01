// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Types.internal.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <opentxs/protobuf/ContractEnums.pb.h>
#include <limits>

#include "opentxs/AddressType.hpp"  // IWYU pragma: keep

namespace opentxs
{
static constexpr auto address_type_to_proto_ = [] {
    using enum AddressType;
    using enum protobuf::AddressType;

    return frozen::make_unordered_map<AddressType, protobuf::AddressType>({
        {Error, ADDRESSTYPE_ERROR},
        {IPV4, ADDRESSTYPE_IPV4},
        {IPV6, ADDRESSTYPE_IPV6},
        {Onion2, ADDRESSTYPE_ONION},
        {EEP, ADDRESSTYPE_EEP},
        {Inproc, ADDRESSTYPE_INPROC},
    });
}();
}  // namespace opentxs

namespace opentxs
{
auto check_subset(
    std::size_t size,
    std::size_t target,
    std::size_t pos) noexcept -> bool
{
    if (pos > size) { return false; }

    if ((std::numeric_limits<std::size_t>::max() - pos) < target) {

        return false;
    }

    if ((pos + target) > size) { return false; }

    return true;
}

auto translate(AddressType in) noexcept -> protobuf::AddressType
{
    const auto& map = address_type_to_proto_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::ADDRESSTYPE_ERROR;
    }
}
}  // namespace opentxs

namespace opentxs::protobuf
{
auto translate(AddressType in) noexcept -> opentxs::AddressType
{
    static const auto map =
        frozen::invert_unordered_map(address_type_to_proto_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return opentxs::AddressType::Error;
    }
}
}  // namespace opentxs::protobuf
