// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <algorithm>

#pragma once

#include <cstddef>
#include <functional>
#include <optional>

#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
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

class Crypto;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Input;
}  // namespace internal
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)
namespace opentxs::blockchain::bitcoin::block::input
{
using blockchain::block::ElementHash;
using blockchain::block::KeyData;

class Data final : public Allocated
{
public:
    using PubkeyHashes = Set<ElementHash>;

    auto for_each_key(std::function<void(const crypto::Key&)> cb) const noexcept
        -> void;
    auto get_allocator() const noexcept -> allocator_type final
    {
        return previous_output_.get_allocator();
    }
    auto keys(Set<crypto::Key>& out) const noexcept -> void;
    auto net_balance_change(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym,
        const std::size_t index,
        const Log& log) const noexcept -> opentxs::Amount;
    auto payer() const noexcept -> identifier::Generic;
    auto spends() const noexcept(false) -> const block::Output&;

    auto add(crypto::Key&& key) noexcept -> void;
    auto associate(const block::Output& in) noexcept -> bool;
    auto Hashes(std::function<PubkeyHashes()> cb) noexcept -> PubkeyHashes&;
    auto merge(
        const api::Crypto& crypto,
        const internal::Input& rhs,
        const std::size_t index,
        const Log& log) noexcept -> void;
    auto reset_size() noexcept -> void;
    auto ScriptHash(std::function<std::optional<ElementHash>()> cb) noexcept
        -> std::optional<ElementHash>&;
    auto set(const KeyData& data) noexcept -> void;
    template <typename F>
    auto size(const bool normalize, F cb) noexcept -> std::size_t
    {
        auto& output = normalize ? normalized_size_ : size_;

        if (false == output.has_value()) { output = cb(); }

        return output.value();
    }

    Data(
        block::Output output,
        std::optional<std::size_t>&& size,
        Set<crypto::Key>&& keys,
        allocator_type alloc) noexcept;
    Data() = delete;
    Data(const Data& rhs, allocator_type alloc) noexcept;
    Data(const Data&) = delete;

private:
    block::Output previous_output_;
    std::optional<std::size_t> size_;
    std::optional<std::size_t> normalized_size_;
    Set<crypto::Key> keys_;
    identifier::Generic payer_;
    std::optional<std::optional<ElementHash>> script_hash_;
    std::optional<PubkeyHashes> pubkey_hashes_;
};
}  // namespace opentxs::blockchain::bitcoin::block::input
