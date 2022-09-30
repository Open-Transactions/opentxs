// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "blockchain/bitcoin/block/Input.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::bitcoin::block::implementation
{
Input::Cache::Cache(
    std::unique_ptr<internal::Output>&& output,
    std::optional<std::size_t>&& size,
    boost::container::flat_set<crypto::Key>&& keys) noexcept
    : lock_()
    , previous_output_(std::move(output))
    , size_(std::move(size))
    , normalized_size_()
    , keys_(std::move(keys))
    , payer_()
{
}

Input::Cache::Cache(const Cache& rhs) noexcept
    : lock_()
    , previous_output_()
    , size_()
    , normalized_size_()
    , keys_()
    , payer_([&] {
        auto lock = rLock{rhs.lock_};

        return rhs.payer_;
    }())
{
    auto lock = rLock{rhs.lock_};

    if (rhs.previous_output_) {
        previous_output_ = rhs.previous_output_->clone();
    }

    size_ = rhs.size_;
    normalized_size_ = rhs.normalized_size_;
    keys_ = rhs.keys_;
}

auto Input::Cache::add(crypto::Key&& key) noexcept -> void
{
    auto lock = rLock{lock_};
    keys_.emplace(std::move(key));
}

auto Input::Cache::associate(const internal::Output& in) noexcept -> bool
{
    auto lock = rLock{lock_};
    previous_output_ = in.clone();
    auto keys = previous_output_->Keys();

    OT_ASSERT(0 < keys.size());

    std::move(keys.begin(), keys.end(), std::inserter(keys_, keys_.end()));

    return bool(previous_output_);
}

auto Input::Cache::keys() const noexcept -> UnallocatedVector<crypto::Key>
{
    auto lock = rLock{lock_};
    auto output = UnallocatedVector<crypto::Key>{};
    std::transform(
        std::begin(keys_), std::end(keys_), std::back_inserter(output), [
        ](const auto& key) -> auto{ return key; });

    return output;
}

auto Input::Cache::merge(
    const internal::Input& rhs,
    const std::size_t index,
    const Log& log) noexcept -> bool
{
    auto lock = rLock{lock_};

    try {
        auto previous = rhs.Spends().clone();
        log(OT_PRETTY_CLASS())("previous output for input ")(
            index)(" instantiated")
            .Flush();

        if (previous_output_) {
            previous_output_->MergeMetadata(*previous, log);
        } else {
            previous_output_ = std::move(previous);
        }
    } catch (...) {
        log(OT_PRETTY_CLASS())(
            "failed to instantiate previous output for input ")(index)
            .Flush();
    }

    if (previous_output_) {
        auto keys = previous_output_->Keys();
        std::move(keys.begin(), keys.end(), std::inserter(keys_, keys_.end()));
    }

    for (const auto& key : rhs.Keys()) {
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

    return true;
}

auto Input::Cache::net_balance_change(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym,
    const std::size_t index,
    const Log& log) const noexcept -> opentxs::Amount
{
    auto lock = rLock{lock_};

    if (false == bool(previous_output_)) {
        log(OT_PRETTY_CLASS())("previous output data for input ")(
            index)(" is missing, possibly because the input is not known to "
                   "belong to any nym in this wallet")
            .Flush();

        return 0;
    }

    for (const auto& key : keys_) {
        if (crypto.Owner(key) == nym) {
            const auto value = -1 * previous_output_->Value();
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

auto Input::Cache::payer() const noexcept -> identifier::Generic
{
    auto lock = rLock{lock_};

    return payer_;
}

auto Input::Cache::reset_size() noexcept -> void
{
    auto lock = rLock{lock_};
    size_ = std::nullopt;
    normalized_size_ = std::nullopt;
}

auto Input::Cache::set(const blockchain::block::KeyData& data) noexcept -> void
{
    auto lock = rLock{lock_};

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

auto Input::Cache::spends() const noexcept(false) -> const internal::Output&
{
    auto lock = rLock{lock_};

    if (previous_output_) {

        return *previous_output_;
    } else {

        throw std::runtime_error("previous output missing");
    }
}
}  // namespace opentxs::blockchain::bitcoin::block::implementation
