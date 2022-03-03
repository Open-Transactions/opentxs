// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <mutex>
#include <tuple>

#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;
namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::bitcoin
{
class Header final
{
public:
    struct BitcoinFormat {
        MagicField magic_;
        CommandField command_;
        PayloadSizeField length_;
        ChecksumField checksum_;

        auto Checksum() const noexcept -> OTData;
        auto Command() const noexcept -> bitcoin::Command;
        auto Network() const noexcept -> blockchain::Type;
        auto PayloadSize() const noexcept -> std::size_t;

        BitcoinFormat(const Data& in) noexcept(false);
        BitcoinFormat(const zmq::Frame& in) noexcept(false);
        BitcoinFormat(
            const blockchain::Type network,
            const bitcoin::Command command,
            const std::size_t payload,
            const OTData checksum) noexcept(false);

    private:
        BitcoinFormat(const void* data, const std::size_t size) noexcept(false);
    };

    static auto Size() noexcept -> std::size_t { return sizeof(BitcoinFormat); }

    auto Command() const noexcept -> bitcoin::Command { return command_; }
    auto Serialize(const AllocateOutput out) const noexcept -> bool;
    auto Network() const noexcept -> blockchain::Type { return chain_; }
    auto PayloadSize() const noexcept -> std::size_t { return payload_size_; }
    auto Checksum() const noexcept -> const opentxs::Data& { return checksum_; }

    auto SetChecksum(const std::size_t payload, OTData&& checksum) noexcept
        -> void;

    Header(
        const api::Session& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept;
    Header(
        const api::Session& api,
        const blockchain::Type network,
        const bitcoin::Command command,
        const std::size_t payload,
        const OTData checksum) noexcept;

    ~Header() = default;

private:
    static constexpr auto header_size_ = std::size_t{24};

    blockchain::Type chain_;
    bitcoin::Command command_;
    std::size_t payload_size_;
    OTData checksum_;

    Header() = delete;
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin
