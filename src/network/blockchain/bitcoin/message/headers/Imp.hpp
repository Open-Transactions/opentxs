// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::network::blockchain::bitcoin::message::internal::MessagePrivate
// IWYU pragma: no_include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <span>

#include "internal/network/blockchain/bitcoin/message/Headers.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/Imp.hpp"
#include "network/blockchain/bitcoin/message/headers/MessagePrivate.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
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

class ByteArray;
class WriteBuffer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::headers
{
class Message final : public headers::MessagePrivate,
                      public implementation::Message
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* final
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    auto get() const noexcept
        -> std::span<const internal::Headers::value_type> final
    {
        return payload_;
    }
    auto IsValid() const noexcept -> bool final { return true; }

    auto get() noexcept -> std::span<internal::Headers::value_type> final
    {
        return payload_;
    }
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> final
    {
        return make_deleter(this);
    }

    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        Vector<internal::Headers::value_type> payload,
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
    static constexpr auto header_ = 80_uz;

    Vector<internal::Headers::value_type> payload_;
    mutable std::optional<std::size_t> cached_size_;

    auto get_payload(Transport type, WriteBuffer& buf) const noexcept(false)
        -> void final;
    auto get_size() const noexcept -> std::size_t final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::headers