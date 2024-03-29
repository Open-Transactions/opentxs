// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/blockchain/Imp_blockchain.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainTransaction.pb.h>
#include <opentxs/protobuf/StorageThread.pb.h>
#include <opentxs/protobuf/StorageThreadItem.pb.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <sstream>
#include <string_view>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "internal/api/crypto/blockchain/BalanceOracle.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/crypto/Types.internal.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"

namespace opentxs::api::crypto::imp
{
BlockchainImp::BlockchainImp(
    const api::session::Client& api,
    const api::session::Activity& activity,
    const api::session::Contacts& contacts,
    const api::internal::Paths& legacy,
    const std::string_view dataFolder,
    const Options& args,
    api::crypto::Blockchain& parent) noexcept
    : Imp(api, contacts, parent)
    , client_(api)
    , activity_(activity)
    , key_generated_endpoint_(opentxs::network::zeromq::MakeArbitraryInproc())
    , lock_()
    , transaction_updates_([&] {
        auto out = api_.Network().ZeroMQ().Context().Internal().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainTransactions().data());

        assert_true(listen);

        return out;
    }())
    , key_updates_([&] {
        auto out = api_.Network().ZeroMQ().Context().Internal().PublishSocket();
        const auto listen = out->Start(key_generated_endpoint_);

        assert_true(listen);

        return out;
    }())
    , scan_updates_([&] {
        auto out = api_.Network().ZeroMQ().Context().Internal().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainScanProgress().data());

        assert_true(listen);

        return out;
    }())
{
}

auto BlockchainImp::ActivityDescription(
    const identifier::Nym& nym,
    const identifier::Generic& thread,
    const std::string_view itemID,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> UnallocatedCString
{
    auto data = protobuf::StorageThread{};

    if (false == api_.Storage().Internal().Load(nym, thread, data)) {
        LogError()()("thread ")(thread, api_.Crypto())(
            " does not exist for nym ")(nym, api_.Crypto())
            .Flush();

        return {};
    }

    for (const auto& item : data.item()) {
        if (item.id() != itemID) { continue; }

        const auto txid =
            opentxs::blockchain::block::TransactionHash{item.txid()};
        const auto chain = static_cast<opentxs::blockchain::Type>(item.chain());
        const auto tx = LoadTransaction(txid, monotonic, monotonic);

        if (false == tx.IsValid()) {
            LogError()()("failed to load transaction ").asHex(txid).Flush();

            return {};
        }

        return ActivityDescription(nym, chain, tx);
    }

    LogError()()("item ")(itemID)(" not found ").Flush();

    return {};
}

auto BlockchainImp::ActivityDescription(
    const identifier::Nym& nym,
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::Transaction& tx) const noexcept
    -> UnallocatedCString
{
    auto output = std::stringstream{};
    const auto amount = tx.NetBalanceChange(parent_, nym);
    const auto memo = tx.Memo(parent_);
    const auto names = [&] {
        auto out = UnallocatedSet<UnallocatedCString>{};
        // TODO allocator
        const auto contacts = tx.AssociatedRemoteContacts(client_, nym, {});

        for (const auto& id : contacts) {
            out.emplace(contacts_.ContactName(id, blockchain_to_unit(chain)));
        }

        return out;
    }();

    if (0 < amount) {
        output << "Incoming ";
    } else if (0 > amount) {
        output << "Outgoing ";
    }

    output << print(chain);
    output << " transaction";

    if (0 < names.size()) {
        output << " ";

        if (0 < amount) {
            output << "from ";
        } else {
            output << "to ";
        }

        auto n = 0_uz;
        const auto max = names.size();

        for (auto i = names.begin(); i != names.end(); ++n, ++i) {
            if (0_uz == n) {
                output << *i;
            } else if ((n + 1_uz) == max) {
                output << ", and ";
                output << *i;
            } else {
                output << ", ";
                output << *i;
            }
        }
    }

    if (false == memo.empty()) { output << ": " << memo; }

    return output.str();
}

auto BlockchainImp::AssignTransactionMemo(
    const TxidHex& id,
    const std::string_view label,
    alloc::Default monotonic) const noexcept -> bool
{
    auto lock = Lock{lock_};
    auto proto = protobuf::BlockchainTransaction{};
    auto transaction = load_transaction(lock, id, proto, monotonic, monotonic);

    if (false == transaction.IsValid()) {
        LogError()()("transaction ")(label)(" does not exist").Flush();

        return false;
    }

    transaction.Internal().asBitcoin().SetMemo(label);
    const auto& db = api_.Network().Blockchain().Internal().Database();

    if (false == db.StoreTransaction(transaction)) {
        LogError()()("failed to save updated transaction ")(id).Flush();

        return false;
    }

    broadcast_update_signal(proto, transaction);

    return true;
}

auto BlockchainImp::broadcast_update_signal(
    const Txid& txid,
    alloc::Default monotonic) const noexcept -> void
{
    broadcast_update_signal(span_from_object(txid), monotonic);
}

auto BlockchainImp::broadcast_update_signal(
    std::span<const Txid> transactions,
    alloc::Default monotonic) const noexcept -> void
{
    const auto& db = api_.Network().Blockchain().Internal().Database();
    std::ranges::for_each(
        transactions, [this, &db, alloc = monotonic](const auto& txid) {
            auto proto = protobuf::BlockchainTransaction{};
            const auto tx =
                db.LoadTransaction(txid.Bytes(), proto, alloc, alloc);
            broadcast_update_signal(proto, tx);
        });
}

auto BlockchainImp::broadcast_update_signal(
    const protobuf::BlockchainTransaction& proto,
    const opentxs::blockchain::block::Transaction& tx) const noexcept -> void
{
    const auto chains = tx.Chains({});  // TODO allocator
    std::ranges::for_each(chains, [&](const auto& chain) {
        transaction_updates_->Send([&] {
            auto work = opentxs::network::zeromq::tagged_message(
                WorkType::BlockchainNewTransaction, true);
            work.AddFrame(tx.ID());
            work.AddFrame(chain);
            work.Internal().AddFrame(proto);

            return work;
        }());
    });
}

auto BlockchainImp::IndexItem(const ReadView bytes) const noexcept
    -> opentxs::blockchain::block::ElementHash
{
    auto output = opentxs::blockchain::block::ElementHash{};
    const auto hashed = api_.Crypto().Hash().HMAC(
        opentxs::crypto::HashType::SipHash24,
        api_.Network().Blockchain().Internal().Database().HashKey(),
        bytes,
        preallocated(sizeof(output), &output));

    assert_true(hashed);

    return output;
}

auto BlockchainImp::KeyEndpoint() const noexcept -> std::string_view
{
    return key_generated_endpoint_;
}

auto BlockchainImp::KeyGenerated(
    const opentxs::blockchain::crypto::Target target,
    const identifier::Nym& account,
    const identifier::Account& subaccount,
    const opentxs::blockchain::crypto::SubaccountType type,
    const opentxs::blockchain::crypto::Subchain subchain) const noexcept -> void
{
    key_updates_->Send([&] {
        using namespace opentxs::blockchain::crypto;
        auto work = MakeWork(OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL);
        serialize(target, work);    // NOTE index 1, 2, 3
        work.AddFrame(account);     // NOTE index 4
        work.AddFrame(subaccount);  // NOTE index 5
        work.AddFrame(subchain);    // NOTE index 6
        work.AddFrame(type);        // NOTE index 7

        return work;
    }());
}

auto BlockchainImp::LoadTransaction(
    const TxidHex& txid,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept
    -> opentxs::blockchain::block::Transaction
{
    auto lock = Lock{lock_};

    return load_transaction(lock, txid, alloc, monotonic);
}

auto BlockchainImp::LoadTransaction(
    const Txid& txid,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept
    -> opentxs::blockchain::block::Transaction
{
    auto lock = Lock{lock_};

    return load_transaction(lock, txid, alloc, monotonic);
}

auto BlockchainImp::load_transaction(
    const Lock& lock,
    const TxidHex& txid,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept
    -> opentxs::blockchain::block::Transaction
{
    auto proto = protobuf::BlockchainTransaction{};

    return load_transaction(lock, txid, proto, alloc, monotonic);
}

auto BlockchainImp::load_transaction(
    const Lock& lock,
    const TxidHex& txid,
    protobuf::BlockchainTransaction& out,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept
    -> opentxs::blockchain::block::Transaction
{
    return load_transaction(lock, {IsHex, txid}, out, alloc, monotonic);
}

auto BlockchainImp::load_transaction(
    const Lock& lock,
    const Txid& txid,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept
    -> opentxs::blockchain::block::Transaction
{
    auto proto = protobuf::BlockchainTransaction{};

    return load_transaction(lock, txid, proto, alloc, monotonic);
}

auto BlockchainImp::load_transaction(
    const Lock& lock,
    const Txid& txid,
    protobuf::BlockchainTransaction& out,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept
    -> opentxs::blockchain::block::Transaction
{
    return api_.Network().Blockchain().Internal().Database().LoadTransaction(
        txid.Bytes(), out, alloc, monotonic);
}

auto BlockchainImp::LookupContacts(const Data& pubkeyHash) const noexcept
    -> ContactList
{
    return api_.Network().Blockchain().Internal().Database().LookupContact(
        pubkeyHash);
}

auto BlockchainImp::ProcessContact(
    const Contact& contact,
    alloc::Default monotonic) const noexcept -> bool
{
    broadcast_update_signal(
        api_.Network().Blockchain().Internal().Database().UpdateContact(
            contact),
        monotonic);

    return true;
}

auto BlockchainImp::ProcessMergedContact(
    const Contact& parent,
    const Contact& child,
    alloc::Default monotonic) const noexcept -> bool
{
    broadcast_update_signal(
        api_.Network().Blockchain().Internal().Database().UpdateMergedContact(
            parent, child),
        monotonic);

    return true;
}

auto BlockchainImp::ProcessTransactions(
    const opentxs::blockchain::Type chain,
    Set<opentxs::blockchain::block::Transaction>&& in,
    const PasswordPrompt& reason,
    alloc::Default monotonic) const noexcept -> bool
{
    const auto& db = api_.Network().Blockchain().Internal().Database();
    const auto& log = LogTrace();
    auto lock = Lock{lock_};

    for (const auto& tx : in) {
        const auto& id = tx.ID();
        const auto txid = id.Bytes();
        auto old = db.LoadTransaction(txid, monotonic, monotonic);
        auto proto = protobuf::BlockchainTransaction{};

        if (old.IsValid()) {
            old.Internal().asBitcoin().MergeMetadata(
                parent_, chain, tx.Internal().asBitcoin(), log);

            if (false == db.StoreTransaction(old, proto)) {
                LogError()()("failed to save updated transaction ")(id.asHex())
                    .Flush();

                return false;
            }
        } else {
            if (false == db.StoreTransaction(tx, proto)) {
                LogError()()("failed to save new transaction ")(id.asHex())
                    .Flush();

                return false;
            }
        }

        // TODO allocator
        if (false ==
            db.AssociateTransaction(
                id, tx.Internal().asBitcoin().IndexElements(api_, {}))) {
            LogError()()("associate patterns for transaction ")(id.asHex())
                .Flush();

            return false;
        }

        if (!reconcile_contact_activities(
                lock, proto, (old.IsValid() ? old : tx))) {

            return false;
        }
    }

    return true;
}

auto BlockchainImp::reconcile_contact_activities(
    const Lock& lock,
    const Txid& txid,
    alloc::Default monotonic) const noexcept -> bool
{
    auto proto = protobuf::BlockchainTransaction{};
    const auto tx = load_transaction(lock, txid, proto, monotonic, monotonic);

    if (false == tx.IsValid()) { return false; }

    return reconcile_contact_activities(lock, proto, tx);
}

auto BlockchainImp::reconcile_contact_activities(
    const Lock& lock,
    const protobuf::BlockchainTransaction& proto,
    const opentxs::blockchain::block::Transaction& tx) const noexcept -> bool
{
    if (!activity_.AddBlockchainTransaction(parent_, tx)) { return false; }

    broadcast_update_signal(proto, tx);

    return true;
}

auto BlockchainImp::ReportScan(
    const opentxs::blockchain::Type chain,
    const identifier::Nym& owner,
    const opentxs::blockchain::crypto::SubaccountType type,
    const identifier::Account& id,
    const Blockchain::Subchain subchain,
    const opentxs::blockchain::block::Position& progress) const noexcept -> void
{
    assert_false(owner.empty());
    assert_false(id.empty());

    const auto hash = progress.hash_.Bytes();
    scan_updates_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::BlockchainWalletScanProgress, true);
        work.AddFrame(chain);
        work.AddFrame(owner.data(), owner.size());
        work.AddFrame(type);
        id.Serialize(work);
        work.AddFrame(subchain);
        work.AddFrame(progress.height_);
        work.AddFrame(hash.data(), hash.size());

        return work;
    }());
}

auto BlockchainImp::Start(
    std::shared_ptr<const api::internal::Session> api) noexcept -> void
{
    Imp::Start(api);
    blockchain::BalanceOracle{std::move(api), balance_oracle_endpoint_}.Start();
}

auto BlockchainImp::Unconfirm(
    const Blockchain::Key key,
    const opentxs::blockchain::block::TransactionHash& txid,
    const Time time,
    alloc::Default monotonic) const noexcept -> bool
{
    auto& alloc = monotonic;
    auto out = Imp::Unconfirm(key, txid, time, alloc);
    auto lock = Lock{lock_};

    if (auto tx = load_transaction(lock, txid, alloc, alloc); tx.IsValid()) {
        static const auto null = opentxs::blockchain::block::Position{};
        tx.Internal().asBitcoin().SetMinedPosition(null);
        const auto& db = api_.Network().Blockchain().Internal().Database();

        if (false == db.StoreTransaction(tx)) {
            LogError()()("failed to save updated transaction ")(txid.asHex())
                .Flush();

            return false;
        }
    }

    return out;
}

auto BlockchainImp::UpdateElement(
    std::span<const ReadView> hashes,
    alloc::Default monotonic) const noexcept -> void
{
    const auto patterns = [&] {
        auto out = Vector<opentxs::blockchain::block::ElementHash>{monotonic};
        out.reserve(hashes.size());
        out.clear();
        std::ranges::transform(
            hashes, std::back_inserter(out), [&](const auto& val) {
                return IndexItem(val);
            });

        return out;
    }();
    const auto transactions = [&] {
        auto out =
            Vector<opentxs::blockchain::block::TransactionHash>{monotonic};
        out.reserve(patterns.size());
        out.clear();
        std::ranges::for_each(patterns, [&](const auto& pattern) {
            auto matches = api_.Network()
                               .Blockchain()
                               .Internal()
                               .Database()
                               .LookupTransactions(pattern);
            out.reserve(out.size() + matches.size());
            std::ranges::move(matches, std::back_inserter(out));
        });
        dedup(out);

        return out;
    }();
    auto lock = Lock{lock_};
    std::ranges::for_each(transactions, [&](const auto& txid) {
        reconcile_contact_activities(lock, txid, monotonic);
    });
}
}  // namespace opentxs::api::crypto::imp
