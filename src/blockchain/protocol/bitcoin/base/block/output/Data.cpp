// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/protocol/bitcoin/base/block/output/Imp.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>

#include "internal/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::implementation
{
Output::Cache::Cache(
    std::optional<std::size_t>&& size,
    Set<crypto::Key>&& keys,
    block::Position&& minedPosition,
    node::TxoState state,
    UnallocatedSet<node::TxoTag>&& tags) noexcept
    : lock_()
    , size_(std::move(size))
    , payee_()
    , payer_()
    , keys_(std::move(keys))
    , mined_position_(std::move(minedPosition))
    , state_(state)
    , tags_(std::move(tags))
{
}

Output::Cache::Cache(const Cache& rhs) noexcept
    : lock_()
    , size_()
    , payee_([&] {
        auto lock = Lock{rhs.lock_};

        return rhs.payee_;
    }())
    , payer_([&] {
        auto lock = Lock{rhs.lock_};

        return rhs.payer_;
    }())
    , keys_()
    , mined_position_([&] {
        auto lock = Lock{rhs.lock_};

        return rhs.mined_position_;
    }())
    , state_()
    , tags_()
{
    auto lock = Lock{rhs.lock_};
    size_ = rhs.size_;
    keys_ = rhs.keys_;
    state_ = rhs.state_;
    tags_ = rhs.tags_;
}

auto Output::Cache::add(crypto::Key key) noexcept -> void
{
    const auto& [account, subchain, index] = key;

    if (blockchain::crypto::Subchain::Outgoing == subchain) { OT_FAIL; }

    auto lock = Lock{lock_};
    keys_.emplace(std::move(key));
}

auto Output::Cache::add(node::TxoTag tag) noexcept -> void
{
    auto lock = Lock{lock_};
    tags_.emplace(tag);
}

auto Output::Cache::keys(Set<crypto::Key>& out) const noexcept -> void
{
    auto lock = Lock{lock_};
    std::copy(keys_.begin(), keys_.end(), std::inserter(out, out.end()));
}

auto Output::Cache::payee() const noexcept -> identifier::Generic
{
    auto lock = Lock{lock_};

    return payee_;
}

auto Output::Cache::merge(
    const api::Crypto& crypto,
    const internal::Output& rhs,
    const std::size_t index,
    const Log& log) noexcept -> void
{
    for (const auto& key : rhs.Keys({})) {  // TODO alloc
        const auto& [account, subchain, idx] = key;

        if (crypto::Subchain::Outgoing == subchain) {
            LogError()(OT_PRETTY_CLASS())("discarding invalid key").Flush();
        } else {
            add(key);
        }
    }

    if (auto p = rhs.Payer(); payer_.empty() || false == p.empty()) {
        set_payer(std::move(p));
        log(OT_PRETTY_CLASS())("setting payer for output ")(index)(" to ")(
            payer_, crypto)
            .Flush();
    }

    if (auto p = rhs.Payee(); payee_.empty() || false == p.empty()) {
        set_payee(std::move(p));
        log(OT_PRETTY_CLASS())("setting payee for output ")(index)(" to ")(
            payee_, crypto)
            .Flush();
    }

    mined_position_ = rhs.MinedPosition();
    state_ = rhs.State();
    const auto tags = rhs.Tags();
    std::copy(tags.begin(), tags.end(), std::inserter(tags_, tags_.end()));
}

auto Output::Cache::payer() const noexcept -> identifier::Generic
{
    auto lock = Lock{lock_};

    return payer_;
}

auto Output::Cache::position() const noexcept -> const block::Position&
{
    auto lock = Lock{lock_};

    return mined_position_;
}

auto Output::Cache::reset_size() noexcept -> void
{
    auto lock = Lock{lock_};
    size_ = std::nullopt;
}

auto Output::Cache::set(const KeyData& data) noexcept -> void
{
    auto lock = Lock{lock_};
    const auto havePayee = [&] { return !payee_.empty(); };
    const auto havePayer = [&] { return !payer_.empty(); };

    for (const auto& key : keys_) {
        if (havePayee() && havePayer()) { return; }

        try {
            const auto& [sender, recipient] = data.at(key);

            if (false == sender.empty()) {
                if (payer_.empty()) { payer_ = sender; }
            }

            if (false == recipient.empty()) {
                if (payee_.empty()) { payee_ = recipient; }
            }
        } catch (...) {
        }
    }
}

auto Output::Cache::set_payee(const identifier::Generic& contact) noexcept
    -> void
{
    set_payee(identifier::Generic{contact});
}

auto Output::Cache::set_payee(identifier::Generic&& contact) noexcept -> void
{
    auto lock = Lock{lock_};
    payee_.Assign(contact);
}

auto Output::Cache::set_payer(const identifier::Generic& contact) noexcept
    -> void
{
    set_payer(identifier::Generic{contact});
}

auto Output::Cache::set_payer(identifier::Generic&& contact) noexcept -> void
{
    auto lock = Lock{lock_};
    payer_.Assign(contact);
}

auto Output::Cache::set_position(const block::Position& pos) noexcept -> void
{
    auto lock = Lock{lock_};
    mined_position_ = pos;
}

auto Output::Cache::set_state(node::TxoState state) noexcept -> void
{
    auto lock = Lock{lock_};
    state_ = state;
}

auto Output::Cache::state() const noexcept -> node::TxoState
{
    auto lock = Lock{lock_};

    return state_;
}

auto Output::Cache::tags() const noexcept -> UnallocatedSet<node::TxoTag>
{
    auto lock = Lock{lock_};

    return tags_;
}
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::base::block::implementation
