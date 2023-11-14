// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::network::blockchain::bitcoin::message::internal::MessagePrivate
// IWYU pragma: no_include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#pragma once

#include <cstddef>
#include <optional>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/Imp.hpp"
#include "network/blockchain/bitcoin/message/reject/MessagePrivate.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::reject
{
class Message final : public reject::MessagePrivate,
                      public implementation::Message
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* final
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    auto IsValid() const noexcept -> bool final { return true; }
    auto Reason() const noexcept -> ReadView final { return reason_; }
    auto RejectedMessage() const noexcept -> ReadView final { return reason_; }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        ReadView name,
        std::byte code,
        ReadView reason,
        ReadView extra,
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
    const CString name_;
    const CString reason_;
    const std::byte code_;
    const ByteArray extra_;

    static auto extra_data(ReadView name, std::byte code) noexcept
        -> std::size_t;

    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        ReadView name,
        ReadView& payload,
        allocator_type alloc) noexcept(false);
    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        ReadView name,
        std::byte code,
        ReadView& payload,
        allocator_type alloc) noexcept(false);
};
}  // namespace opentxs::network::blockchain::bitcoin::message::reject
