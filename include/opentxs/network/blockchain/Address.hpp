// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

namespace network
{
namespace blockchain
{
namespace internal
{
class Address;
}  // namespace internal

class Address;
class AddressPrivate;
}  // namespace blockchain
}  // namespace network

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::network::blockchain::Address> {
    using is_avalanching = void;

    auto operator()(const opentxs::network::blockchain::Address& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::network::blockchain
{
OPENTXS_EXPORT auto operator==(const Address&, const Address&) noexcept -> bool;
OPENTXS_EXPORT auto operator<=>(const Address&, const Address&) noexcept
    -> std::strong_ordering;
}  // namespace opentxs::network::blockchain

namespace opentxs::network::blockchain
{
class OPENTXS_EXPORT Address
{
public:
    auto Bytes() const noexcept -> ByteArray;
    auto Chain() const noexcept -> opentxs::blockchain::Type;
    auto Display() const noexcept -> UnallocatedCString;
    auto ID() const noexcept -> const identifier::Generic&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Address&;
    auto IsValid() const noexcept -> bool;
    auto Key() const noexcept -> ReadView;
    auto LastConnected() const noexcept -> Time;
    auto Port() const noexcept -> std::uint16_t;
    auto Services() const noexcept -> Set<bitcoin::Service>;
    auto Style() const noexcept -> Protocol;
    auto Subtype() const noexcept -> Transport;
    auto Type() const noexcept -> Transport;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Address&;

    OPENTXS_NO_EXPORT Address(AddressPrivate* imp) noexcept;
    Address() noexcept;
    Address(const Address&) noexcept;
    Address(Address&&) noexcept;
    auto operator=(const Address&) noexcept -> Address&;
    auto operator=(Address&&) noexcept -> Address&;

    ~Address();

private:
    AddressPrivate* imp_;
};
}  // namespace opentxs::network::blockchain
