// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <utility>
#include <vector>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"

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

namespace internal
{
struct Address;
}  // namespace internal
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs

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
            const p2p::internal::Address& address);
        BitcoinFormat_31402();
    };

    using pAddress = std::unique_ptr<blockchain::p2p::internal::Address>;
    using AddressVector = std::pmr::vector<pAddress>;

    static auto ExtractAddress(AddressByteField in) noexcept
        -> std::pair<p2p::Network, OTData>;
    static auto SerializeTimestamp(const ProtocolVersion version) noexcept
        -> bool;

    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return *payload_.at(position);
    }
    auto begin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto end() const noexcept -> const_iterator final
    {
        return const_iterator(this, payload_.size());
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

    ~Addr() final = default;

private:
    const ProtocolVersion version_;
    const AddressVector payload_;

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;

    Addr(const Addr&) = delete;
    Addr(Addr&&) = delete;
    auto operator=(const Addr&) -> Addr& = delete;
    auto operator=(Addr&&) -> Addr& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
