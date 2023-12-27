// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/subaccount/ethereum/Imp.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string_view>

#include "blockchain/crypto/element/Element.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/crypto/Element.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Bip44Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.internal.hpp"
#include "opentxs/blockchain/protocol/ethereum/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Role.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::crypto
{
using namespace std::literals;

EthereumPrivate::EthereumPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const identifier::Account& id,
    const proto::HDPath& path,
    const HDProtocol standard,
    const PasswordPrompt& reason,
    opentxs::crypto::SeedID seed) noexcept(false)
    : ImportedPrivate(
          api,
          parent,
          id,
          [&] {
              const auto key = api.Crypto().Seed().Internal().AccountKey(
                  path, Bip44Subchain::external, reason);

              if (false == key.IsValid()) {

                  throw std::runtime_error{"Failed to derive account key"};
              }

              return key.ChildKey(index_, reason);
          }(),
          seed,
          api.Crypto().Seed().SeedDescription(seed),
          get_name(path, standard))
    , path_(path)
    , standard_(standard)
    , version_(default_version_)
    , balance_()
    , next_()
    , known_()
    , incoming_()
    , outgoing_()
    , self_()
{
    init(false);
}

static auto to_txid(std::string_view in) noexcept -> block::TransactionHash
{
    return in;
}

EthereumPrivate::EthereumPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const identifier::Account& id,
    const SerializedType& proto,
    opentxs::crypto::SeedID seed) noexcept(false)
    : ImportedPrivate(
          api,
          parent,
          id,
          proto.imported(),
          seed,
          api.Crypto().Seed().SeedDescription(seed),
          get_name(proto.path(), static_cast<HDProtocol>(proto.standard())))
    , path_(proto.path())
    , standard_(static_cast<HDProtocol>(proto.standard()))
    , version_(default_version_)
    , balance_(protocol::ethereum::native_to_amount(proto.balance())
                   .value_or(Amount{}))
    , next_(proto.next_nonce())
    , known_([&] {
        auto out = decltype(known_){};
        std::ranges::copy(
            proto.known_outgoing(), std::inserter(out, out.end()));

        return out;
    }())
    , incoming_([&] {
        auto out = decltype(incoming_){};
        std::ranges::transform(
            proto.incoming_txid(), std::inserter(out, out.end()), to_txid);

        return out;
    }())
    , outgoing_([&] {
        auto out = decltype(outgoing_){};
        std::ranges::transform(
            proto.outgoing_txid(), std::inserter(out, out.end()), to_txid);

        return out;
    }())
    , self_()
{
    init(true);
}

auto EthereumPrivate::account_already_exists(const rLock& lock) const noexcept
    -> bool
{
    const auto existing =
        api_.Storage().Internal().BlockchainEthereumAccountList(
            parent_.NymID(), target_to_unit(target_));

    return existing.contains(id_);
}

auto EthereumPrivate::AddIncoming(
    const Amount& balance,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    auto lock = rLock{lock_};

    if (add_incoming(lock, balance, txid, confirmed)) {

        return save(lock);
    } else {

        return false;
    }
}

auto EthereumPrivate::AddIncoming(
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    auto lock = rLock{lock_};

    if (add_incoming(lock, txid, confirmed)) {

        return save(lock);
    } else {

        return false;
    }
}

auto EthereumPrivate::add_incoming(
    const rLock& lock,
    const Amount& balance,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    if (add_incoming(lock, txid, confirmed)) {

        return update_balance(lock, balance);
    } else {

        return false;
    }
}

auto EthereumPrivate::add_incoming(
    const rLock& lock,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    incoming_.emplace(txid);

    if (confirmed) {

        return element_.Internal().Confirm(txid);
    } else {

        return element_.Internal().Unconfirm(txid, Clock::now());
    }
}

auto EthereumPrivate::AddOutgoing(
    const Amount& balance,
    protocol::ethereum::AccountNonce nonce,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    auto lock = rLock{lock_};

    if (add_outgoing(lock, balance, nonce, txid, confirmed)) {

        return save(lock);
    } else {

        return false;
    }
}

auto EthereumPrivate::AddOutgoing(
    protocol::ethereum::AccountNonce nonce,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    auto lock = rLock{lock_};

    if (add_outgoing(lock, nonce, txid, confirmed)) {

        return save(lock);
    } else {

        return false;
    }
}

auto EthereumPrivate::add_outgoing(
    const rLock& lock,
    const Amount& balance,
    protocol::ethereum::AccountNonce nonce,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    if (add_outgoing(lock, nonce, txid, confirmed)) {

        return update_balance(lock, balance);
    } else {

        return false;
    }
}

auto EthereumPrivate::add_outgoing(
    const rLock& lock,
    protocol::ethereum::AccountNonce nonce,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    known_.emplace(nonce);
    outgoing_.emplace(txid);

    if (confirmed) {

        return element_.Internal().Confirm(txid);
    } else {

        return element_.Internal().Unconfirm(txid, Clock::now());
    }
}

auto EthereumPrivate::Balance() const noexcept -> Amount
{
    auto lock = rLock{lock_};

    return balance_;
}

auto EthereumPrivate::InitSelf(std::shared_ptr<Subaccount> me) noexcept -> void
{
    self_.emplace(me);
}

auto EthereumPrivate::KnownIncoming(alloc::Strategy alloc) const noexcept
    -> Set<block::TransactionHash>
{
    auto out = Set<block::TransactionHash>{alloc.result_};
    auto lock = rLock{lock_};
    std::ranges::copy(incoming_, std::inserter(out, out.end()));

    return out;
}

auto EthereumPrivate::KnownOutgoing(alloc::Strategy alloc) const noexcept
    -> Set<block::TransactionHash>
{
    auto out = Set<block::TransactionHash>{alloc.result_};
    auto lock = rLock{lock_};
    std::ranges::copy(outgoing_, std::inserter(out, out.end()));

    return out;
}

auto EthereumPrivate::MissingOutgoing(alloc::Strategy alloc) const noexcept
    -> Set<protocol::ethereum::AccountNonce>
{
    using namespace std::ranges;
    using protocol::ethereum::AccountNonce;
    auto out = Set<AccountNonce>{alloc.result_};
    constexpr auto start = AccountNonce{0U};

    if (start < next_) {
        const auto expected = iota_view{start, next_ - 1U};
        set_difference(expected, known_, std::inserter(out, out.end()));
    }

    return out;
}

auto EthereumPrivate::NextOutgoing() const noexcept
    -> protocol::ethereum::AccountNonce
{
    auto lock = rLock{lock_};

    return next_;
}

auto EthereumPrivate::save(const rLock& lock) const noexcept -> bool
{
    try {
        auto proto = SerializedType{};
        const auto serialize_known = [&proto](const auto& nonce) {
            proto.add_known_outgoing(nonce);
        };
        const auto serialize_incoming = [&proto](const auto& id) {
            proto.add_incoming_txid()->assign(id.Bytes());
        };
        const auto serialize_outgoing = [&proto](const auto& id) {
            proto.add_outgoing_txid()->assign(id.Bytes());
        };
        proto.set_version(version_);
        serialize_imported(lock, *proto.mutable_imported());

        {
            using namespace protocol::ethereum;
            auto& out = *proto.mutable_balance();

            if (false == amount_to_native(balance_, writer(out))) {
                throw std::runtime_error{"failed to serialize amount"};
            }
        }

        proto.set_next_nonce(next_);
        using namespace std::ranges;
        for_each(known_, serialize_known);
        for_each(incoming_, serialize_incoming);
        for_each(outgoing_, serialize_outgoing);
        const auto saved = api_.Storage().Internal().Store(
            parent_.NymID(), target_to_unit(target_), proto);

        if (false == saved) {

            throw std::runtime_error{
                "failed to save "s.append(print(parent_.Target()))
                    .append(" ethereum subaccount ")
                    .append(id_.asBase58(api_.Crypto()))};
        }

        LogTrace()()("Saved ")(print(parent_.Target()))(
            " ethereum subaccount ")(id_, api_.Crypto())
            .Flush();

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto EthereumPrivate::UpdateBalance(const Amount& balance) noexcept -> bool
{
    auto lock = rLock{lock_};

    return update_balance(lock, balance);
}

auto EthereumPrivate::update_balance(
    const rLock& lock,
    const Amount& balance) noexcept -> bool
{
    balance_ = balance;

    return save(lock);
}

EthereumPrivate::~EthereumPrivate() = default;
}  // namespace opentxs::blockchain::crypto
