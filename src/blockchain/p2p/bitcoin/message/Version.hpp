// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <set>
#include <string>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Time.hpp"

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
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Version final : public internal::Version, public implementation::Message
{
public:
    struct BitcoinFormat_1 {
        ProtocolVersionFieldSigned version_{};
        BitVectorField services_{};
        TimestampField64 timestamp_{};
        AddressVersion remote_{};

        BitcoinFormat_1(
            const bitcoin::ProtocolVersion version,
            const std::pmr::set<bitcoin::Service>& localServices,
            const std::pmr::set<bitcoin::Service>& remoteServices,
            const tcp::endpoint& remoteAddress,
            const Time time) noexcept;
        BitcoinFormat_1() noexcept;
    };

    struct BitcoinFormat_106 {
        AddressVersion local_{};
        NonceField nonce_{};

        BitcoinFormat_106(
            const std::pmr::set<bitcoin::Service>& localServices,
            const tcp::endpoint localAddress,
            const bitcoin::Nonce nonce) noexcept;
        BitcoinFormat_106() noexcept;
    };

    struct BitcoinFormat_209 {
        HeightField height_{};

        BitcoinFormat_209(const block::Height height) noexcept;
        BitcoinFormat_209() noexcept;
    };

    auto Height() const noexcept -> block::Height final { return height_; }
    auto LocalAddress() const noexcept -> tcp::endpoint final
    {
        return local_address_;
    }
    auto LocalServices() const noexcept
        -> std::pmr::set<blockchain::p2p::Service> final
    {
        return local_services_;
    }
    auto Nonce() const noexcept -> bitcoin::Nonce final { return nonce_; }
    auto ProtocolVersion() const noexcept -> bitcoin::ProtocolVersion final
    {
        return version_;
    }
    auto Relay() const noexcept -> bool final { return relay_; }
    auto RemoteAddress() const noexcept -> tcp::endpoint final
    {
        return remote_address_;
    }
    auto RemoteServices() const noexcept
        -> std::pmr::set<blockchain::p2p::Service> final
    {
        return remote_services_;
    }
    auto UserAgent() const noexcept -> const std::string& final
    {
        return user_agent_;
    }

    Version(
        const api::Session& api,
        const blockchain::Type network,
        const bitcoin::ProtocolVersion version,
        const tcp::endpoint localAddress,
        const tcp::endpoint remoteAddress,
        const std::pmr::set<blockchain::p2p::Service>& services,
        const std::pmr::set<blockchain::p2p::Service>& localServices,
        const std::pmr::set<blockchain::p2p::Service>& remoteServices,
        const bitcoin::Nonce nonce,
        const std::string& userAgent,
        const block::Height height,
        const bool relay,
        const Time time = Clock::now()) noexcept;
    Version(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const bitcoin::ProtocolVersion version,
        const tcp::endpoint localAddress,
        const tcp::endpoint remoteAddress,
        const std::pmr::set<blockchain::p2p::Service>& services,
        const std::pmr::set<blockchain::p2p::Service>& localServices,
        const std::pmr::set<blockchain::p2p::Service>& remoteServices,
        const bitcoin::Nonce nonce,
        const std::string& userAgent,
        const block::Height height,
        const bool relay,
        const Time time = Clock::now()) noexcept;

    ~Version() final = default;

private:
    const bitcoin::ProtocolVersion version_;
    const tcp::endpoint local_address_;
    const tcp::endpoint remote_address_;
    const std::pmr::set<blockchain::p2p::Service> services_;
    const std::pmr::set<blockchain::p2p::Service> local_services_;
    const std::pmr::set<blockchain::p2p::Service> remote_services_;
    const bitcoin::Nonce nonce_;
    const std::string user_agent_;
    const block::Height height_;
    const bool relay_;
    const Time timestamp_;

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;

    Version(const Version&) = delete;
    Version(Version&&) = delete;
    auto operator=(const Version&) -> Version& = delete;
    auto operator=(Version&&) -> Version& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
