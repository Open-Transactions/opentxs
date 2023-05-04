// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/input/Data.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/bitcoin/block/Input.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::bitcoin::block::input
{
Data::Data(
    block::Output output,
    std::optional<std::size_t>&& size,
    Set<crypto::Key>&& keys,
    allocator_type alloc) noexcept
    : previous_output_(std::move(output), alloc)
    , size_(std::move(size))
    , normalized_size_()
    , keys_(std::move(keys), alloc)
    , payer_(alloc)
    , script_hash_()
    , pubkey_hashes_()
{
}

Data::Data(const Data& rhs, allocator_type alloc) noexcept
    : previous_output_(rhs.previous_output_, alloc)
    , size_(rhs.size_)
    , normalized_size_(rhs.normalized_size_)
    , keys_(rhs.keys_, alloc)
    , payer_(rhs.payer_, alloc)
    , script_hash_(rhs.script_hash_)
    , pubkey_hashes_([&]() -> std::optional<PubkeyHashes> {
        if (rhs.pubkey_hashes_.has_value()) {

            return PubkeyHashes{*rhs.pubkey_hashes_, alloc};
        } else {

            return std::nullopt;
        }
    }())
{
}

auto Data::add(crypto::Key&& key) noexcept -> void
{
    keys_.emplace(std::move(key));
}

auto Data::associate(const block::Output& in) noexcept -> bool
{
    previous_output_ = in;
    auto keys = previous_output_.Keys(get_allocator());

    OT_ASSERT(0 < keys.size());

    std::move(keys.begin(), keys.end(), std::inserter(keys_, keys_.end()));

    return previous_output_.IsValid();
}

auto Data::Hashes(std::function<PubkeyHashes()> cb) noexcept -> PubkeyHashes&
{
    if (false == pubkey_hashes_.has_value()) {
        OT_ASSERT(cb);

        pubkey_hashes_.emplace(std::invoke(cb));
    }

    return *pubkey_hashes_;
}

auto Data::keys(Set<crypto::Key>& out) const noexcept -> void
{
    std::copy(keys_.begin(), keys_.end(), std::inserter(out, out.end()));
}

auto Data::merge(
    const internal::Input& rhs,
    const std::size_t index,
    const Log& log) noexcept -> void
{
    try {
        const auto& previous = rhs.Spends();
        log(OT_PRETTY_CLASS())("previous output for input ")(
            index)(" instantiated")
            .Flush();

        if (previous_output_.IsValid()) {
            previous_output_.Internal().MergeMetadata(previous.Internal(), log);
        } else {
            previous_output_ = previous;
        }
    } catch (...) {
        log(OT_PRETTY_CLASS())(
            "failed to instantiate previous output for input ")(index)
            .Flush();
    }

    if (previous_output_.IsValid()) {
        auto keys = previous_output_.Keys(get_allocator());
        std::move(keys.begin(), keys.end(), std::inserter(keys_, keys_.end()));
    }

    for (const auto& key : rhs.Keys(get_allocator())) {
        if (0u == keys_.count(key)) {
            log(OT_PRETTY_CLASS())("adding key ")(print(key))(" to input ")(
                index)
                .Flush();
        } else {
            log(OT_PRETTY_CLASS())("input ")(
                index)(" is already associated with ")(print(key))
                .Flush();
        }
    }
}

auto Data::net_balance_change(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym,
    const std::size_t index,
    const Log& log) const noexcept -> opentxs::Amount
{
    if (false == previous_output_.IsValid()) {
        log(OT_PRETTY_CLASS())("previous output data for input ")(
            index)(" is missing, possibly because the input is not known to "
                   "belong to any nym in this wallet")
            .Flush();

        return 0;
    }

    for (const auto& key : keys_) {
        if (crypto.Owner(key) == nym) {
            const auto value = -1 * previous_output_.Value();
            log(OT_PRETTY_CLASS())("input ")(index)(" contributes ")(value)
                .Flush();

            return value;
        } else {
            log(OT_PRETTY_CLASS())("input ")(
                index)(" belongs to a different nym")
                .Flush();
        }
    }

    if (0 == keys_.size()) {
        log(OT_PRETTY_CLASS())("no keys are associated with input ")(
            index)(" even though the previous output data is present")
            .Flush();
    }

    return 0;
}

auto Data::payer() const noexcept -> identifier::Generic { return payer_; }

auto Data::reset_size() noexcept -> void
{
    size_ = std::nullopt;
    normalized_size_ = std::nullopt;
}

auto Data::ScriptHash(std::function<std::optional<ElementHash>()> cb) noexcept
    -> std::optional<ElementHash>&
{
    if (false == script_hash_.has_value()) {
        OT_ASSERT(cb);

        script_hash_.emplace(std::invoke(cb));
    }

    return *script_hash_;
}

auto Data::set(const KeyData& data) noexcept -> void
{
    if (payer_.empty()) {
        for (const auto& key : keys_) {
            try {
                const auto& [sender, recipient] = data.at(key);

                if (recipient.empty()) { continue; }

                payer_ = recipient;

                return;
            } catch (...) {
            }
        }
    }
}

auto Data::spends() const noexcept(false) -> const block::Output&
{
    if (previous_output_.IsValid()) {

        return previous_output_;
    } else {

        throw std::runtime_error("previous output missing");
    }
}
}  // namespace opentxs::blockchain::bitcoin::block::input
