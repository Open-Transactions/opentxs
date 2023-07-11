// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::network::blockchain::bitcoin::message::internal::MessagePrivate
// IWYU pragma: no_include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#pragma once

#include <cstddef>
#include <optional>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/Imp.hpp"
#include "network/blockchain/bitcoin/message/version/MessagePrivate.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class WriteBuffer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::version
{
class Message final : public version::MessagePrivate,
                      public implementation::Message
{
public:
    struct BitcoinFormat_1 {
        ProtocolVersionFieldSigned version_;
        BitVectorField services_;
        TimestampField64 timestamp_;
        AddressVersion remote_;

        BitcoinFormat_1(
            const message::ProtocolVersion version,
            const UnallocatedSet<message::Service>& localServices,
            const UnallocatedSet<message::Service>& remoteServices,
            const tcp::endpoint& remoteAddress,
            const Time time) noexcept;
        BitcoinFormat_1() noexcept;
    };

    struct BitcoinFormat_106 {
        AddressVersion local_;
        NonceField nonce_;

        BitcoinFormat_106(
            const UnallocatedSet<message::Service>& localServices,
            const tcp::endpoint localAddress,
            const message::Nonce nonce) noexcept;
        BitcoinFormat_106() noexcept;
    };

    struct BitcoinFormat_209 {
        HeightField height_;

        BitcoinFormat_209(
            const opentxs::blockchain::block::Height height) noexcept;
        BitcoinFormat_209() noexcept;
    };

    auto Bip37() const noexcept -> bool final { return bip37_; }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* final
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    auto Height() const noexcept -> opentxs::blockchain::block::Height final
    {
        return height_;
    }
    auto IsValid() const noexcept -> bool final { return true; }
    auto LocalAddress() const noexcept -> tcp::endpoint final
    {
        return local_address_;
    }
    auto LocalServices(allocator_type alloc) const noexcept
        -> Set<bitcoin::Service> final
    {
        return local_services_;
    }
    auto Nonce() const noexcept -> message::Nonce final { return nonce_; }
    auto ProtocolVersion() const noexcept -> message::ProtocolVersion final
    {
        return version_;
    }
    auto RemoteAddress() const noexcept -> tcp::endpoint final
    {
        return remote_address_;
    }
    auto RemoteServices(allocator_type alloc) const noexcept
        -> Set<bitcoin::Service> final
    {
        return remote_services_;
    }
    auto UserAgent() const noexcept -> ReadView final { return user_agent_; }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }

    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        message::ProtocolVersion version,
        tcp::endpoint localAddress,
        tcp::endpoint remoteAddress,
        Set<network::blockchain::bitcoin::Service> services,
        Set<network::blockchain::bitcoin::Service> localServices,
        Set<network::blockchain::bitcoin::Service> remoteServices,
        message::Nonce nonce,
        CString userAgent,
        opentxs::blockchain::block::Height height,
        bool bip37,
        Time timestamp,
        std::optional<ByteArray> avalanche,
        std::optional<ByteArray> dash,
        allocator_type alloc) noexcept;
    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        ReadView& payload,
        allocator_type alloc) noexcept(false);
    Message(const Message& rhs, allocator_type alloc) noexcept;
    Message(const Message&) = delete;
    Message(Message&&) = delete;
    auto operator=(const Message&) -> Message& = delete;
    auto operator=(Message&&) -> Message& = delete;

    ~Message() final = default;

private:
    const message::ProtocolVersion version_;
    const tcp::endpoint local_address_;
    const tcp::endpoint remote_address_;
    const Set<network::blockchain::bitcoin::Service> services_;
    const Set<network::blockchain::bitcoin::Service> local_services_;
    const Set<network::blockchain::bitcoin::Service> remote_services_;
    const message::Nonce nonce_;
    const CString user_agent_;
    const opentxs::blockchain::block::Height height_;
    const bool bip37_;
    const Time timestamp_;
    const std::optional<ByteArray> avalanche_;
    const std::optional<ByteArray> dash_extra_data_;
    mutable std::optional<std::size_t> cached_size_;

    auto get_payload(Transport type, WriteBuffer& buf) const noexcept(false)
        -> void final;
    auto get_size() const noexcept -> std::size_t final;

    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        ReadView& payload,
        BitcoinFormat_1 data,
        allocator_type alloc) noexcept(false);
    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        ReadView& payload,
        message::ProtocolVersion version,
        Set<network::blockchain::bitcoin::Service> services,
        tcp::endpoint remoteAddress,
        Set<network::blockchain::bitcoin::Service> remoteServices,
        Time timestamp,
        std::pair<BitcoinFormat_106, CString> data,
        allocator_type alloc) noexcept(false);
    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        ReadView& payload,
        message::ProtocolVersion version,
        Set<network::blockchain::bitcoin::Service> services,
        tcp::endpoint remoteAddress,
        Set<network::blockchain::bitcoin::Service> remoteServices,
        Time timestamp,
        tcp::endpoint localAddress,
        Set<network::blockchain::bitcoin::Service> localServices,
        message::Nonce nonce,
        CString userAgent,
        const BitcoinFormat_209& data,
        allocator_type alloc) noexcept(false);
};
}  // namespace opentxs::network::blockchain::bitcoin::message::version
