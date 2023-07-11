// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>
#include <string_view>

#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::transaction
{
class Data final : public Allocated
{
public:
    auto chains(allocator_type alloc) const noexcept -> Set<blockchain::Type>;
    auto get_allocator() const noexcept -> allocator_type final;
    auto height() const noexcept -> block::Height;
    auto memo() const noexcept -> std::string_view;
    auto position() const noexcept -> const block::Position&;

    auto add(blockchain::Type chain) noexcept -> void;
    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }
    auto merge(
        const api::crypto::Blockchain& crypto,
        const internal::Transaction& rhs,
        const Log& log) noexcept -> void;
    template <typename F>
    auto normalized(F cb) noexcept -> const identifier::Generic&
    {
        auto& output = normalized_id_;

        if (false == output.has_value()) { output = cb(); }

        return output.value();
    }
    auto reset_size() noexcept -> void;
    auto set_memo(const std::string_view memo) noexcept -> void;
    auto set_memo(UnallocatedCString&& memo) noexcept -> void;
    auto set_position(const block::Position& pos) noexcept -> void;
    template <typename F>
    auto size(const bool normalize, F cb) noexcept -> std::size_t
    {
        auto& output = normalize ? normalized_size_ : size_;

        if (false == output.has_value()) { output = cb(); }

        return output.value();
    }

    Data(
        std::string_view memo,
        Set<blockchain::Type> chains,
        block::Position minedPosition,
        allocator_type alloc) noexcept(false);
    Data() = delete;
    Data(const Data& rhs, allocator_type alloc) noexcept;
    Data(const Data&) noexcept = delete;
    Data(Data&&) = delete;
    auto operator=(const Data&) -> Data& = delete;
    auto operator=(Data&&) -> Data& = delete;

    ~Data() final;

private:
    std::optional<identifier::Generic> normalized_id_;
    std::optional<std::size_t> size_;
    std::optional<std::size_t> normalized_size_;
    CString memo_;
    Set<blockchain::Type> chains_;
    block::Position mined_position_;
};
}  // namespace opentxs::blockchain::bitcoin::block::transaction
