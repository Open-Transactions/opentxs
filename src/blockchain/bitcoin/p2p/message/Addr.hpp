// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <utility>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin

class Address;
}  // namespace p2p
}  // namespace blockchain

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Addr final : public internal::Addr, public implementation::Message
{
public:
    struct BitcoinFormat_31402 {
        TimestampField32 time_;
        AddressVersion data_;

        BitcoinFormat_31402(
            const blockchain::Type chain,
            const ProtocolVersion version,
            const p2p::Address& address);
        BitcoinFormat_31402();
    };

    using AddressVector = UnallocatedVector<blockchain::p2p::Address>;

    static auto ExtractAddress(AddressByteField in) noexcept
        -> std::pair<p2p::Network, ByteArray>;
    static auto SerializeTimestamp(const ProtocolVersion version) noexcept
        -> bool;

    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return payload_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return {this, 0}; }
    auto end() const noexcept -> const_iterator final
    {
        return {this, payload_.size()};
    }
    auto SerializeTimestamp() const noexcept -> bool
    {
        return SerializeTimestamp(version_);
    }
    auto size() const noexcept -> std::size_t final { return payload_.size(); }

    Addr(
        const api::Session& api,
        const blockchain::Type network,
        const ProtocolVersion version,
        AddressVector&& addresses) noexcept;
    Addr(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const ProtocolVersion version,
        AddressVector&& addresses) noexcept;
    Addr(const Addr&) = delete;
    Addr(Addr&&) = delete;
    auto operator=(const Addr&) -> Addr& = delete;
    auto operator=(Addr&&) -> Addr& = delete;

    ~Addr() final = default;

private:
    const ProtocolVersion version_;
    const AddressVector payload_;

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
