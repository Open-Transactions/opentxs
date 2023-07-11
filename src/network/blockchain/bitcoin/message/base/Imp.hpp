// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/network/blockchain/bitcoin/message/Header.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class WriteBuffer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::implementation
{
class Message : virtual public internal::MessagePrivate
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* override
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    auto Command() const noexcept -> message::Command final { return command_; }
    auto Describe() const noexcept -> ReadView final { return description_; }
    auto Network() const noexcept -> opentxs::blockchain::Type final
    {
        return chain_;
    }
    auto Transmit(Transport type, zeromq::Message& out) const noexcept(false)
        -> void final;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return make_deleter(this);
    }

    Message(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        message::Command command,
        std::optional<ByteArray> checksum,
        allocator_type alloc) noexcept;
    Message(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        ReadView description,
        std::optional<ByteArray> checksum,
        allocator_type alloc) noexcept;
    Message() = delete;
    Message(const Message&) = delete;
    Message(Message&&) = delete;
    Message(const Message& rhs, allocator_type alloc) noexcept;
    auto operator=(const Message&) -> Message& = delete;
    auto operator=(Message&&) -> Message& = delete;

    ~Message() override = default;

protected:
    const api::Session& api_;
    const opentxs::blockchain::Type chain_;
    const message::Command command_;
    const CString description_;

    static constexpr auto verify_hash_size(std::size_t size) noexcept -> bool
    {
        return size == opentxs::blockchain::standard_hash_size_;
    }

private:
    mutable std::optional<ByteArray> checksum_;

    virtual auto get_payload(Transport type, WriteBuffer& buf) const
        noexcept(false) -> void;
    virtual auto get_size() const noexcept -> std::size_t;
    auto header(const ReadView payload) const noexcept -> internal::Header;
    auto transmit_asio(Transport type, zeromq::Message& out) const
        noexcept(false) -> void;
    auto transmit_zmq(Transport type, zeromq::Message& out) const
        noexcept(false) -> void;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::implementation
