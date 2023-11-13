// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::network::blockchain::bitcoin::message::internal::MessagePrivate
// IWYU pragma: no_include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/Imp.hpp"
#include "network/blockchain/bitcoin/message/cfilter/MessagePrivate.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
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

namespace opentxs::network::blockchain::bitcoin::message::cfilter
{
class Message final : public cfilter::MessagePrivate,
                      public implementation::Message
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> internal::MessagePrivate* final
    {
        return pmr::clone_as<internal::MessagePrivate>(this, {alloc});
    }
    auto Bits() const noexcept -> std::uint8_t final { return params_.first; }
    auto ElementCount() const noexcept -> std::uint32_t final { return count_; }
    auto FPRate() const noexcept -> std::uint32_t final
    {
        return params_.second;
    }
    auto Filter() const noexcept -> ReadView final { return filter_.Bytes(); }
    auto Hash() const noexcept -> const opentxs::blockchain::block::Hash& final
    {
        return hash_;
    }
    auto IsValid() const noexcept -> bool final { return true; }
    auto Type() const noexcept -> opentxs::blockchain::cfilter::Type final
    {
        return type_;
    }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        opentxs::blockchain::cfilter::Type type,
        opentxs::blockchain::block::Hash hash,
        std::uint32_t count,
        ByteArray compressed,
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
    static constexpr auto prefix_ = sizeof(FilterPrefixBasic);

    const opentxs::blockchain::cfilter::Type type_;
    const opentxs::blockchain::block::Hash hash_;
    const std::uint32_t count_;
    const ByteArray filter_;
    const opentxs::blockchain::internal::FilterParams params_;
    mutable std::optional<std::size_t> cached_size_;

    auto get_payload(Transport type, WriteBuffer& buf) const noexcept(false)
        -> void final;
    auto get_size() const noexcept -> std::size_t final;

    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        ReadView& payload,
        FilterPrefixBasic data,
        allocator_type alloc) noexcept(false);
    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        opentxs::blockchain::cfilter::Type type,
        opentxs::blockchain::block::Hash hash,
        ReadView cfilter,
        allocator_type alloc) noexcept(false);
    Message(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        std::optional<ByteArray> checksum,
        opentxs::blockchain::cfilter::Type type,
        opentxs::blockchain::block::Hash hash,
        std::uint32_t count,
        ReadView& compressed,
        allocator_type alloc) noexcept(false);
};
}  // namespace opentxs::network::blockchain::bitcoin::message::cfilter
