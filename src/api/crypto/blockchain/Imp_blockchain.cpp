// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/blockchain/Imp_blockchain.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <StorageThread.pb.h>
#include <StorageThreadItem.pb.h>
#include <algorithm>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <sstream>
#include <string_view>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "internal/api/crypto/blockchain/BalanceOracle.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"
#include "util/Work.hpp"

namespace opentxs::api::crypto::imp
{
BlockchainImp::BlockchainImp(
    const api::session::Client& api,
    const api::session::Activity& activity,
    const api::session::Contacts& contacts,
    const api::Legacy& legacy,
    const std::string_view dataFolder,
    const Options& args,
    api::crypto::Blockchain& parent) noexcept
    : Imp(api, contacts, parent)
    , client_(api)
    , activity_(activity)
    , key_generated_endpoint_(opentxs::network::zeromq::MakeArbitraryInproc())
    , transaction_updates_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainTransactions().data());

        OT_ASSERT(listen);

        return out;
    }())
    , key_updates_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PublishSocket();
        const auto listen = out->Start(key_generated_endpoint_);

        OT_ASSERT(listen);

        return out;
    }())
    , scan_updates_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainScanProgress().data());

        OT_ASSERT(listen);

        return out;
    }())
    , new_blockchain_accounts_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PublishSocket();
        const auto listen =
            out->Start(api_.Endpoints().BlockchainAccountCreated().data());

        OT_ASSERT(listen);

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
    auto data = proto::StorageThread{};

    if (false ==
        api_.Storage().Load(nym, thread.asBase58(api_.Crypto()), data)) {
        LogError()(OT_PRETTY_CLASS())("thread ")(
            thread)(" does not exist for nym ")(nym)
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
            LogError()(OT_PRETTY_CLASS())("failed to load transaction ")
                .asHex(txid)
                .Flush();

            return {};
        }

        return ActivityDescription(nym, chain, tx);
    }

    LogError()(OT_PRETTY_CLASS())("item ")(itemID)(" not found ").Flush();

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
            out.emplace(contacts_.ContactName(id, BlockchainToUnit(chain)));
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
    auto proto = proto::BlockchainTransaction{};
    auto transaction = load_transaction(lock, id, proto, monotonic, monotonic);

    if (false == transaction.IsValid()) {
        LogError()(OT_PRETTY_CLASS())("transaction ")(label)(" does not exist")
            .Flush();

        return false;
    }

    transaction.Internal().asBitcoin().SetMemo(label);
    const auto& db = api_.Network().Blockchain().Internal().Database();

    if (false == db.StoreTransaction(transaction)) {
        LogError()(OT_PRETTY_CLASS())("failed to save updated transaction ")(id)
            .Flush();

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
    std::for_each(
        std::begin(transactions),
        std::end(transactions),
        [this, &db, alloc = monotonic](const auto& txid) {
            auto proto = proto::BlockchainTransaction{};
            const auto tx =
                db.LoadTransaction(txid.Bytes(), proto, alloc, alloc);
            broadcast_update_signal(proto, tx);
        });
}

auto BlockchainImp::broadcast_update_signal(
    const proto::BlockchainTransaction& proto,
    const opentxs::blockchain::block::Transaction& tx) const noexcept -> void
{
    const auto chains = tx.Chains({});  // TODO allocator
    std::for_each(std::begin(chains), std::end(chains), [&](const auto& chain) {
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

    OT_ASSERT(hashed);

    return output;
}

auto BlockchainImp::KeyEndpoint() const noexcept -> std::string_view
{
    return key_generated_endpoint_;
}

auto BlockchainImp::KeyGenerated(
    const opentxs::blockchain::Type chain,
    const identifier::Nym& account,
    const identifier::Account& subaccount,
    const opentxs::blockchain::crypto::SubaccountType type,
    const opentxs::blockchain::crypto::Subchain subchain) const noexcept -> void
{
    key_updates_->Send([&] {
        auto work = MakeWork(OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL);
        work.AddFrame(chain);
        work.AddFrame(account);
        work.AddFrame(subaccount);
        work.AddFrame(subchain);
        work.AddFrame(type);

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
    auto proto = proto::BlockchainTransaction{};

    return load_transaction(lock, txid, proto, alloc, monotonic);
}

auto BlockchainImp::load_transaction(
    const Lock& lock,
    const TxidHex& txid,
    proto::BlockchainTransaction& out,
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
    auto proto = proto::BlockchainTransaction{};

    return load_transaction(lock, txid, proto, alloc, monotonic);
}

auto BlockchainImp::load_transaction(
    const Lock& lock,
    const Txid& txid,
    proto::BlockchainTransaction& out,
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

auto BlockchainImp::notify_new_account(
    const identifier::Account& id,
    const identifier::Nym& owner,
    opentxs::blockchain::Type chain,
    opentxs::blockchain::crypto::SubaccountType type) const noexcept -> void
{
    new_blockchain_accounts_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::BlockchainAccountCreated, true);
        work.AddFrame(chain);
        work.AddFrame(owner);
        work.AddFrame(type);
        id.Internal().Serialize(work);

        return work;
    }());
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
        auto proto = proto::BlockchainTransaction{};

        if (old.IsValid()) {
            old.Internal().asBitcoin().MergeMetadata(
                parent_, chain, tx.Internal().asBitcoin(), log);

            if (false == db.StoreTransaction(old, proto)) {
                LogError()(OT_PRETTY_CLASS())(
                    "failed to save updated transaction ")(id.asHex())
                    .Flush();

                return false;
            }
        } else {
            if (false == db.StoreTransaction(tx, proto)) {
                LogError()(OT_PRETTY_CLASS())(
                    "failed to save new transaction ")(id.asHex())
                    .Flush();

                return false;
            }
        }

        // TODO allocator
        if (false ==
            db.AssociateTransaction(
                id, tx.Internal().asBitcoin().IndexElements(api_, {}))) {
            LogError()(OT_PRETTY_CLASS())(
                "associate patterns for transaction ")(id.asHex())
                .Flush();

            return false;
        }

        if (!reconcile_activity_threads(
                lock, proto, (old.IsValid() ? old : tx))) {

            return false;
        }
    }

    return true;
}

auto BlockchainImp::reconcile_activity_threads(
    const Lock& lock,
    const Txid& txid,
    alloc::Default monotonic) const noexcept -> bool
{
    auto proto = proto::BlockchainTransaction{};
    const auto tx = load_transaction(lock, txid, proto, monotonic, monotonic);

    if (false == tx.IsValid()) { return false; }

    return reconcile_activity_threads(lock, proto, tx);
}

auto BlockchainImp::reconcile_activity_threads(
    const Lock& lock,
    const proto::BlockchainTransaction& proto,
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
    OT_ASSERT(false == owner.empty());
    OT_ASSERT(false == id.empty());

    const auto hash = progress.hash_.Bytes();
    scan_updates_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::BlockchainWalletScanProgress, true);
        work.AddFrame(chain);
        work.AddFrame(owner.data(), owner.size());
        work.AddFrame(type);
        id.Internal().Serialize(work);
        work.AddFrame(subchain);
        work.AddFrame(progress.height_);
        work.AddFrame(hash.data(), hash.size());

        return work;
    }());
}

auto BlockchainImp::Start(std::shared_ptr<const api::Session> api) noexcept
    -> void
{
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
            LogError()(OT_PRETTY_CLASS())(
                "failed to save updated transaction ")(txid.asHex())
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
        std::transform(
            hashes.begin(),
            hashes.end(),
            std::back_inserter(out),
            [&](const auto& val) { return IndexItem(val); });

        return out;
    }();
    const auto transactions = [&] {
        auto out =
            Vector<opentxs::blockchain::block::TransactionHash>{monotonic};
        out.reserve(patterns.size());
        out.clear();
        std::for_each(
            patterns.begin(), patterns.end(), [&](const auto& pattern) {
                auto matches = api_.Network()
                                   .Blockchain()
                                   .Internal()
                                   .Database()
                                   .LookupTransactions(pattern);
                out.reserve(out.size() + matches.size());
                std::move(
                    std::begin(matches),
                    std::end(matches),
                    std::back_inserter(out));
            });
        dedup(out);

        return out;
    }();
    auto lock = Lock{lock_};
    std::for_each(
        std::begin(transactions),
        std::end(transactions),
        [&](const auto& txid) {
            reconcile_activity_threads(lock, txid, monotonic);
        });
}
}  // namespace opentxs::api::crypto::imp
