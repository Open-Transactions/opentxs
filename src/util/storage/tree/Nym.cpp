// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Nym.hpp"  // IWYU pragma: associated

#include <BlockchainAccountData.pb.h>
#include <BlockchainDeterministicAccountData.pb.h>
#include <BlockchainEthereumAccountData.pb.h>
#include <BlockchainImportedAccountData.pb.h>
#include <Enums.pb.h>
#include <HDAccount.pb.h>
#include <Nym.pb.h>
#include <Purse.pb.h>
#include <StorageBlockchainAccountList.pb.h>
#include <StorageItemHash.pb.h>
#include <StorageNym.pb.h>
#include <StoragePurse.pb.h>
#include <functional>
#include <source_location>
#include <stdexcept>
#include <variant>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/BlockchainEthereumAccountData.hpp"
#include "internal/serialization/protobuf/verify/HDAccount.hpp"
#include "internal/serialization/protobuf/verify/Nym.hpp"
#include "internal/serialization/protobuf/verify/Purse.hpp"
#include "internal/serialization/protobuf/verify/StorageNym.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Bip47Channels.hpp"
#include "util/storage/tree/Contexts.hpp"
#include "util/storage/tree/Issuers.hpp"
#include "util/storage/tree/Mailbox.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/PaymentWorkflows.hpp"
#include "util/storage/tree/PeerReplies.hpp"
#include "util/storage/tree/PeerRequests.hpp"
#include "util/storage/tree/Thread.hpp"  // IWYU pragma: keep
#include "util/storage/tree/Threads.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

template <>
void Nym::_save(
    tree::Threads* input,
    const Lock& lock,
    std::mutex& mutex,
    Hash& root)
{
    assert_false(nullptr == mail_inbox_);
    assert_false(nullptr == mail_outbox_);

    _save(mail_inbox_.get(), lock, mail_inbox_lock_, mail_inbox_root_);
    _save(mail_outbox_.get(), lock, mail_outbox_lock_, mail_outbox_root_);

    if (nullptr == input) {
        LogError()()("Null target.").Flush();
        LogAbort()().Abort();
    }

    auto rootLock = Lock{mutex};
    root = input->Root();
    rootLock.unlock();

    if (false == save(lock)) {
        LogError()()("Save error.").Flush();
        LogAbort()().Abort();
    }
}

Nym::Nym(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const identifier::Nym& id,
    const Hash& hash,
    std::string_view alias)
    : Node(
          crypto,
          factory,
          storage,
          hash,
          std::source_location::current().function_name(),
          current_version_)
    , alias_(alias)
    , nymid_(id)
    , credentials_(NullHash{})
    , checked_(Flag::Factory(false))
    , private_(Flag::Factory(false))
    , revision_(0)
    , bip47_lock_{}
    , bip47_{nullptr}
    , bip47_root_{NullHash{}}
    , sent_request_box_lock_()
    , sent_request_box_(nullptr)
    , sent_peer_request_(NullHash{})
    , incoming_request_box_lock_()
    , incoming_request_box_(nullptr)
    , incoming_peer_request_(NullHash{})
    , sent_reply_box_lock_()
    , sent_reply_box_(nullptr)
    , sent_peer_reply_(NullHash{})
    , incoming_reply_box_lock_()
    , incoming_reply_box_(nullptr)
    , incoming_peer_reply_(NullHash{})
    , finished_request_box_lock_()
    , finished_request_box_(nullptr)
    , finished_peer_request_(NullHash{})
    , finished_reply_box_lock_()
    , finished_reply_box_(nullptr)
    , finished_peer_reply_(NullHash{})
    , processed_request_box_lock_()
    , processed_request_box_(nullptr)
    , processed_peer_request_(NullHash{})
    , processed_reply_box_lock_()
    , processed_reply_box_(nullptr)
    , processed_peer_reply_(NullHash{})
    , mail_inbox_lock_()
    , mail_inbox_(nullptr)
    , mail_inbox_root_(NullHash{})
    , mail_outbox_lock_()
    , mail_outbox_(nullptr)
    , mail_outbox_root_(NullHash{})
    , threads_lock_()
    , threads_(nullptr)
    , threads_root_(NullHash{})
    , contexts_lock_()
    , contexts_(nullptr)
    , contexts_root_(NullHash{})
    , blockchain_lock_()
    , blockchain_account_types_()
    , blockchain_account_index_()
    , blockchain_accounts_()
    , issuers_root_(NullHash{})
    , issuers_lock_()
    , issuers_(nullptr)
    , workflows_root_(NullHash{})
    , workflows_lock_()
    , workflows_(nullptr)
    , purse_id_()
    , ethereum_lock_()
    , ethereum_account_types_()
    , ethereum_account_index_()
    , ethereum_accounts_()
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

auto Nym::Alias() const -> UnallocatedCString { return alias_; }

auto Nym::bip47() const -> tree::Bip47Channels*
{
    return construct<tree::Bip47Channels>(bip47_lock_, bip47_, bip47_root_);
}

auto Nym::Bip47Channels() const -> const tree::Bip47Channels&
{
    return *bip47();
}

auto Nym::BlockchainAccountList(const UnitType type) const
    -> UnallocatedSet<identifier::Account>
{
    const auto lock = Lock{blockchain_lock_};

    auto it = blockchain_account_types_.find(type);

    if (blockchain_account_types_.end() == it) { return {}; }

    return it->second;
}

auto Nym::BlockchainEthereumAccountList(const UnitType type) const
    -> UnallocatedSet<identifier::Account>
{
    const auto lock = Lock{ethereum_lock_};

    auto it = ethereum_account_types_.find(type);

    if (ethereum_account_types_.end() == it) { return {}; }

    return it->second;
}

auto Nym::BlockchainAccountType(const identifier::Account& accountID) const
    -> UnitType
{
    const auto lock = Lock{blockchain_lock_};

    try {

        return blockchain_account_index_.at(accountID);
    } catch (...) {

        return UnitType::Error;
    }
}

auto Nym::BlockchainEthereumAccountType(
    const identifier::Account& accountID) const -> UnitType
{
    const auto lock = Lock{ethereum_lock_};

    try {

        return ethereum_account_index_.at(accountID);
    } catch (...) {

        return UnitType::Error;
    }
}

template <typename T, typename... Args>
auto Nym::construct(
    std::mutex& mutex,
    std::unique_ptr<T>& pointer,
    const Hash& root,
    Args&&... params) const -> T*
{
    auto lock = Lock{mutex};

    if (false == bool(pointer)) {
        pointer.reset(new T(crypto_, factory_, plugin_, root, params...));

        if (!pointer) {
            LogError()()("Unable to instantiate.").Flush();
            LogAbort()().Abort();
        }
    }

    lock.unlock();

    return pointer.get();
}

auto Nym::contexts() const -> tree::Contexts*
{
    return construct<tree::Contexts>(contexts_lock_, contexts_, contexts_root_);
}

auto Nym::Contexts() const -> const tree::Contexts& { return *contexts(); }

auto Nym::dump(const Lock& lock, const Log& log, Vector<Hash>& out)
    const noexcept -> bool
{
    if (false == is_valid(root_)) { return true; }

    if (false == Node::dump(lock, log, out)) { return false; }

    out.reserve(out.size() + purse_id_.size() + 1_uz);

    for (const auto& [_, hash] : purse_id_) {
        log()(name_)("adding purse hash ")(hash).Flush();
        out.emplace_back(hash);
    }

    if (is_valid(credentials_)) {
        log()(name_)("adding credential hash ")(credentials_).Flush();
        out.emplace_back(credentials_);
    }

    if (is_valid(bip47_root_)) {
        if (false == bip47()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(sent_peer_request_)) {
        if (false == sent_request_box()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(incoming_peer_request_)) {
        if (false == incoming_request_box()->dump(lock, log, out)) {
            return false;
        }
    }

    if (is_valid(sent_peer_reply_)) {
        if (false == sent_reply_box()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(incoming_peer_reply_)) {
        if (false == incoming_reply_box()->dump(lock, log, out)) {
            return false;
        }
    }

    if (is_valid(finished_peer_request_)) {
        if (false == finished_request_box()->dump(lock, log, out)) {
            return false;
        }
    }

    if (is_valid(finished_peer_reply_)) {
        if (false == finished_reply_box()->dump(lock, log, out)) {
            return false;
        }
    }

    if (is_valid(processed_peer_request_)) {
        if (false == processed_request_box()->dump(lock, log, out)) {
            return false;
        }
    }

    if (is_valid(processed_peer_reply_)) {
        if (false == processed_reply_box()->dump(lock, log, out)) {
            return false;
        }
    }

    if (is_valid(mail_inbox_root_)) {
        if (false == mail_inbox()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(mail_outbox_root_)) {
        if (false == mail_outbox()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(threads_root_)) {
        if (false == threads()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(contexts_root_)) {
        if (false == contexts()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(issuers_root_)) {
        if (false == issuers()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(workflows_root_)) {
        if (false == workflows()->dump(lock, log, out)) { return false; }
    }

    return true;
}

template <typename T>
auto Nym::editor(Hash& root, std::mutex& mutex, T* (Nym::*get)() const)
    -> Editor<T>
{
    const std::function<void(T*, Lock&)> callback =
        [&](T* in, Lock& lock) -> void { this->_save(in, lock, mutex, root); };

    return Editor<T>(write_lock_, (this->*get)(), callback);
}

auto Nym::finished_reply_box() const -> PeerReplies*
{
    return construct<tree::PeerReplies>(
        finished_reply_box_lock_, finished_reply_box_, finished_peer_reply_);
}

auto Nym::finished_request_box() const -> PeerRequests*
{
    return construct<tree::PeerRequests>(
        finished_request_box_lock_,
        finished_request_box_,
        finished_peer_request_);
}

auto Nym::FinishedRequestBox() const -> const PeerRequests&
{
    return *finished_request_box();
}

auto Nym::FinishedReplyBox() const -> const PeerReplies&
{
    return *finished_reply_box();
}

auto Nym::incoming_reply_box() const -> PeerReplies*
{
    return construct<tree::PeerReplies>(
        incoming_reply_box_lock_, incoming_reply_box_, incoming_peer_reply_);
}

auto Nym::incoming_request_box() const -> PeerRequests*
{
    return construct<tree::PeerRequests>(
        incoming_request_box_lock_,
        incoming_request_box_,
        incoming_peer_request_);
}

auto Nym::IncomingRequestBox() const -> const PeerRequests&
{
    return *incoming_request_box();
}

auto Nym::IncomingReplyBox() const -> const PeerReplies&
{
    return *incoming_reply_box();
}

auto Nym::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StorageNym>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;
        switch (set_original_version(proto.version())) {
            case 10u: {
                for (const auto& it : proto.ethereum_hd_index()) {
                    const auto& id = it.id();
                    const auto type = ClaimToUnit(translate(id));
                    auto& accountSet = ethereum_account_types_[type];

                    for (const auto& base58 : it.list()) {
                        const auto accountID =
                            factory_.AccountIDFromBase58(base58);
                        accountSet.emplace(accountID);
                        ethereum_account_index_.emplace(accountID, type);
                    }
                }

                for (const auto& account : proto.ethereum_hd()) {
                    ethereum_accounts_.emplace(
                        factory_.AccountIDFromBase58(
                            account.imported().common().id()),
                        std::make_shared<proto::BlockchainEthereumAccountData>(
                            account));
                }

                [[fallthrough]];
            }
            case 9u: {
                // NOTE txo field is no longer used
                [[fallthrough]];
            }
            case 8u: {
                for (const auto& purse : proto.purse()) {
                    auto server = factory_.NotaryIDFromBase58(purse.notary());
                    auto unit = factory_.UnitIDFromBase58(purse.unit());
                    PurseID id{std::move(server), std::move(unit)};
                    purse_id_.emplace(
                        std::move(id), read(purse.purse().hash()));
                }
                [[fallthrough]];
            }
            case 7u: {
                bip47_root_ = read(proto.bip47());
                [[fallthrough]];
            }
            case 6u: {
                workflows_root_ = read(proto.paymentworkflow());
                [[fallthrough]];
            }
            case 5u: {
                issuers_root_ = read(proto.issuers());
                [[fallthrough]];
            }
            case 4u: {
                for (const auto& it : proto.bitcoin_hd_index()) {
                    const auto& id = it.id();
                    const auto type = ClaimToUnit(translate(id));
                    auto& accountSet = blockchain_account_types_[type];

                    for (const auto& base58 : it.list()) {
                        const auto accountID =
                            factory_.AccountIDFromBase58(base58);
                        accountSet.emplace(accountID);
                        blockchain_account_index_.emplace(accountID, type);
                    }
                }

                for (const auto& account : proto.bitcoin_hd()) {
                    blockchain_accounts_.emplace(
                        factory_.AccountIDFromBase58(
                            account.deterministic().common().id()),
                        std::make_shared<proto::HDAccount>(account));
                }

                [[fallthrough]];
            }
            case 3u: {
                if (proto.has_contexts()) {
                    contexts_root_ = read(proto.contexts().hash());
                } else {
                    contexts_root_ = NullHash{};
                }

                [[fallthrough]];
            }
            case 2u: {
                if (proto.has_mailinbox()) {
                    mail_inbox_root_ = read(proto.mailinbox().hash());
                } else {
                    mail_inbox_root_ = NullHash{};
                }

                if (proto.has_mailoutbox()) {
                    mail_outbox_root_ = read(proto.mailoutbox().hash());
                } else {
                    mail_outbox_root_ = NullHash{};
                }

                if (proto.has_threads()) {
                    threads_root_ = read(proto.threads().hash());
                } else {
                    threads_root_ = NullHash{};
                }

                [[fallthrough]];
            }
            case 1u:
            default: {
                if (nymid_ != factory_.IdentifierFromBase58(proto.nymid())) {
                    LogAbort()()("nym id mismatch").Abort();
                }

                credentials_ = read(proto.credlist().hash());
                sent_peer_request_ = read(proto.sentpeerrequests().hash());
                incoming_peer_request_ =
                    read(proto.incomingpeerrequests().hash());
                sent_peer_reply_ = read(proto.sentpeerreply().hash());
                incoming_peer_reply_ = read(proto.incomingpeerreply().hash());
                finished_peer_request_ =
                    read(proto.finishedpeerrequest().hash());
                finished_peer_reply_ = read(proto.finishedpeerreply().hash());
                processed_peer_request_ =
                    read(proto.processedpeerrequest().hash());
                processed_peer_reply_ = read(proto.processedpeerreply().hash());
            }
        }
    } else {
        throw std::runtime_error{"failed to load root object file in "s.append(
            std::source_location::current().function_name())};
    }
}

auto Nym::issuers() const -> tree::Issuers*
{
    return construct<tree::Issuers>(issuers_lock_, issuers_, issuers_root_);
}

auto Nym::Issuers() const -> const tree::Issuers& { return *issuers(); }

auto Nym::Load(
    const identifier::Account& id,
    std::shared_ptr<proto::BlockchainEthereumAccountData>& output,
    ErrorReporting checking) const -> bool
{
    const auto lock = Lock{ethereum_lock_};

    const auto it = ethereum_accounts_.find(id);

    if (ethereum_accounts_.end() == it) {
        using enum ErrorReporting;

        if (verbose == checking) {
            LogError()()("Account does not exist.").Flush();
        }

        return false;
    }

    output = it->second;

    return bool(output);
}

auto Nym::Load(
    const identifier::Account& id,
    std::shared_ptr<proto::HDAccount>& output,
    ErrorReporting checking) const -> bool
{
    const auto lock = Lock{blockchain_lock_};

    const auto it = blockchain_accounts_.find(id);

    if (blockchain_accounts_.end() == it) {
        using enum ErrorReporting;

        if (verbose == checking) {
            LogError()()("Account does not exist.").Flush();
        }

        return false;
    }

    output = it->second;

    return bool(output);
}

auto Nym::Load(
    std::shared_ptr<proto::Nym>& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const -> bool
{
    const auto lock = Lock{write_lock_};
    using enum ErrorReporting;

    if (!is_valid(credentials_)) {
        if (verbose == checking) {
            LogError()()("Error: nym with id ")(nymid_, crypto_)(
                " has no credentials.")
                .Flush();
        }

        return false;
    }

    alias = alias_;
    checked_->Set(LoadProto(credentials_, output, verbose));

    if (!checked_.get()) { return false; }

    private_->Set(proto::NYM_PRIVATE == output->mode());
    revision_.store(output->revision());

    return true;
}

auto Nym::Load(
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit,
    std::shared_ptr<proto::Purse>& output,
    ErrorReporting checking) const -> bool
{
    const auto lock = Lock{write_lock_};
    const PurseID id{notary, unit};
    const auto it = purse_id_.find(id);
    using enum ErrorReporting;

    if (purse_id_.end() == it) {
        if (verbose == checking) { LogError()()("Purse not found ").Flush(); }

        return false;
    }

    const auto& hash = it->second;

    return LoadProto(hash, output, verbose);
}

auto Nym::mail_inbox() const -> Mailbox*
{
    return construct<tree::Mailbox>(
        mail_inbox_lock_, mail_inbox_, mail_inbox_root_);
}

auto Nym::mail_outbox() const -> Mailbox*
{
    return construct<tree::Mailbox>(
        mail_outbox_lock_, mail_outbox_, mail_outbox_root_);
}

auto Nym::MailInbox() const -> const Mailbox& { return *mail_inbox(); }

auto Nym::MailOutbox() const -> const Mailbox& { return *mail_outbox(); }

auto Nym::mutable_Bip47Channels() -> Editor<tree::Bip47Channels>
{
    return editor<tree::Bip47Channels>(bip47_root_, bip47_lock_, &Nym::bip47);
}

auto Nym::mutable_SentRequestBox() -> Editor<PeerRequests>
{
    return editor<tree::PeerRequests>(
        sent_peer_request_, sent_request_box_lock_, &Nym::sent_request_box);
}

auto Nym::mutable_IncomingRequestBox() -> Editor<PeerRequests>
{
    return editor<tree::PeerRequests>(
        incoming_peer_request_,
        incoming_request_box_lock_,
        &Nym::incoming_request_box);
}

auto Nym::mutable_SentReplyBox() -> Editor<PeerReplies>
{
    return editor<tree::PeerReplies>(
        sent_peer_reply_, sent_reply_box_lock_, &Nym::sent_reply_box);
}

auto Nym::mutable_IncomingReplyBox() -> Editor<PeerReplies>
{
    return editor<tree::PeerReplies>(
        incoming_peer_reply_,
        incoming_reply_box_lock_,
        &Nym::incoming_reply_box);
}

auto Nym::mutable_FinishedRequestBox() -> Editor<PeerRequests>
{
    return editor<tree::PeerRequests>(
        finished_peer_request_,
        finished_request_box_lock_,
        &Nym::finished_request_box);
}

auto Nym::mutable_FinishedReplyBox() -> Editor<PeerReplies>
{
    return editor<tree::PeerReplies>(
        finished_peer_reply_,
        finished_reply_box_lock_,
        &Nym::finished_reply_box);
}

auto Nym::mutable_ProcessedRequestBox() -> Editor<PeerRequests>
{
    return editor<tree::PeerRequests>(
        processed_peer_request_,
        processed_request_box_lock_,
        &Nym::processed_request_box);
}

auto Nym::mutable_ProcessedReplyBox() -> Editor<PeerReplies>
{
    return editor<tree::PeerReplies>(
        processed_peer_reply_,
        processed_reply_box_lock_,
        &Nym::processed_reply_box);
}

auto Nym::mutable_MailInbox() -> Editor<Mailbox>
{
    return editor<tree::Mailbox>(
        mail_inbox_root_, mail_inbox_lock_, &Nym::mail_inbox);
}

auto Nym::mutable_MailOutbox() -> Editor<Mailbox>
{
    return editor<tree::Mailbox>(
        mail_outbox_root_, mail_outbox_lock_, &Nym::mail_outbox);
}

auto Nym::mutable_Threads() -> Editor<tree::Threads>
{
    return editor<tree::Threads>(threads_root_, threads_lock_, &Nym::threads);
}

auto Nym::mutable_Threads(
    const blockchain::block::TransactionHash& txid,
    const identifier::Generic& contact,
    const bool add) -> Editor<tree::Threads>
{
    auto* threads = this->threads();

    assert_true(threads);

    if (add) {
        threads->AddIndex(txid, contact);
    } else {
        threads->RemoveIndex(txid, contact);
    }

    auto cb = [&](tree::Threads* in, Lock& lock) {
        _save(in, lock, threads_lock_, threads_root_);
    };

    return {write_lock_, threads, cb};
}

auto Nym::mutable_Contexts() -> Editor<tree::Contexts>
{
    return editor<tree::Contexts>(
        contexts_root_, contexts_lock_, &Nym::contexts);
}

auto Nym::mutable_Issuers() -> Editor<tree::Issuers>
{
    return editor<tree::Issuers>(issuers_root_, issuers_lock_, &Nym::issuers);
}

auto Nym::mutable_PaymentWorkflows() -> Editor<tree::PaymentWorkflows>
{
    return editor<tree::PaymentWorkflows>(
        workflows_root_, workflows_lock_, &Nym::workflows);
}

auto Nym::PaymentWorkflows() const -> const tree::PaymentWorkflows&
{
    return *workflows();
}

auto Nym::processed_reply_box() const -> PeerReplies*
{
    return construct<tree::PeerReplies>(
        processed_reply_box_lock_, processed_reply_box_, processed_peer_reply_);
}

auto Nym::processed_request_box() const -> PeerRequests*
{
    return construct<tree::PeerRequests>(
        processed_request_box_lock_,
        processed_request_box_,
        processed_peer_request_);
}

auto Nym::ProcessedRequestBox() const -> const PeerRequests&
{
    return *processed_request_box();
}

auto Nym::ProcessedReplyBox() const -> const PeerReplies&
{
    return *processed_reply_box();
}

auto Nym::save(const Lock& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogError()()("Lock failure.").Flush();
        LogAbort()().Abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return StoreProto(serialized, root_);
}

template <typename O>
void Nym::_save(O* input, const Lock& lock, std::mutex& mutex, Hash& root)
{
    if (!verify_write_lock(lock)) {
        LogError()()("Lock failure.").Flush();
        LogAbort()().Abort();
    }

    if (nullptr == input) {
        LogError()()("Null target.").Flush();
        LogAbort()().Abort();
    }

    auto rootLock = Lock{mutex};
    root = input->Root();
    rootLock.unlock();

    if (false == save(lock)) {
        LogError()()("Save error.").Flush();
        LogAbort()().Abort();
    }
}

auto Nym::sent_reply_box() const -> PeerReplies*
{
    return construct<tree::PeerReplies>(
        sent_reply_box_lock_, sent_reply_box_, sent_peer_reply_);
}

auto Nym::sent_request_box() const -> PeerRequests*
{
    return construct<tree::PeerRequests>(
        sent_request_box_lock_, sent_request_box_, sent_peer_request_);
}

auto Nym::SentRequestBox() const -> const PeerRequests&
{
    return *sent_request_box();
}

auto Nym::SentReplyBox() const -> const PeerReplies&
{
    return *sent_reply_box();
}

auto Nym::serialize() const -> proto::StorageNym
{
    auto proto = proto::StorageNym{};
    proto.set_version(version_);
    proto.set_nymid(nymid_.asBase58(crypto_));

    set_hash(nymid_, credentials_, *proto.mutable_credlist());
    set_hash(nymid_, sent_peer_request_, *proto.mutable_sentpeerrequests());
    set_hash(
        nymid_, incoming_peer_request_, *proto.mutable_incomingpeerrequests());
    set_hash(nymid_, sent_peer_reply_, *proto.mutable_sentpeerreply());
    set_hash(nymid_, incoming_peer_reply_, *proto.mutable_incomingpeerreply());
    set_hash(
        nymid_, finished_peer_request_, *proto.mutable_finishedpeerrequest());
    set_hash(nymid_, finished_peer_reply_, *proto.mutable_finishedpeerreply());
    set_hash(
        nymid_, processed_peer_request_, *proto.mutable_processedpeerrequest());
    set_hash(
        nymid_, processed_peer_reply_, *proto.mutable_processedpeerreply());
    set_hash(nymid_, mail_inbox_root_, *proto.mutable_mailinbox());
    set_hash(nymid_, mail_outbox_root_, *proto.mutable_mailoutbox());
    set_hash(nymid_, threads_root_, *proto.mutable_threads());
    set_hash(nymid_, contexts_root_, *proto.mutable_contexts());

    for (const auto& it : blockchain_account_types_) {
        const auto& chainType = it.first;
        const auto& accountSet = it.second;
        auto& index = *proto.add_bitcoin_hd_index();
        index.set_version(blockchain_index_version_);
        index.set_id(translate(UnitToClaim(chainType)));

        for (const auto& accountID : accountSet) {
            index.add_list(accountID.asBase58(crypto_));
        }
    }

    for (const auto& it : blockchain_accounts_) {
        assert_false(nullptr == it.second);

        const auto& account = *it.second;
        *proto.add_bitcoin_hd() = account;
    }

    write(issuers_root_, *proto.mutable_issuers());
    write(workflows_root_, *proto.mutable_paymentworkflow());
    write(bip47_root_, *proto.mutable_bip47());

    for (const auto& [key, hash] : purse_id_) {
        const auto& [server, unit] = key;
        auto& purse = *proto.add_purse();
        purse.set_version(storage_purse_version_);
        purse.set_notary(server.asBase58(crypto_));
        purse.set_unit(unit.asBase58(crypto_));
        set_hash(unit, hash, *purse.mutable_purse());
    }

    for (const auto& it : ethereum_account_types_) {
        const auto& chainType = it.first;
        const auto& accountSet = it.second;
        auto& index = *proto.add_ethereum_hd_index();
        index.set_version(blockchain_index_version_);
        index.set_id(translate(UnitToClaim(chainType)));

        for (const auto& accountID : accountSet) {
            index.add_list(accountID.asBase58(crypto_));
        }
    }

    for (const auto& it : ethereum_accounts_) {
        assert_false(nullptr == it.second);

        const auto& account = *it.second;
        *proto.add_ethereum_hd() = account;
    }

    return proto;
}

auto Nym::SetAlias(std::string_view alias) -> bool
{
    const auto lock = Lock{write_lock_};

    alias_ = alias;

    return true;
}

auto Nym::Store(
    const UnitType type,
    const proto::BlockchainEthereumAccountData& data) -> bool
{
    const auto& accountID =
        factory_.AccountIDFromBase58(data.imported().common().id());

    if (accountID.empty()) {
        LogError()()("Invalid account ID.").Flush();

        return false;
    }

    if (false == proto::Validate(data, VERBOSE)) {
        LogError()()("Invalid account.").Flush();

        return false;
    }

    auto writeLock = Lock{write_lock_, std::defer_lock};
    auto ethereumLock = Lock{ethereum_lock_, std::defer_lock};
    std::lock(writeLock, ethereumLock);
    auto accountItem = ethereum_accounts_.find(accountID);

    if (ethereum_accounts_.end() == accountItem) {
        ethereum_accounts_[accountID] =
            std::make_shared<proto::BlockchainEthereumAccountData>(data);
    } else {
        auto& existing = accountItem->second;

        if (existing->imported().common().revision() >
            data.imported().common().revision()) {
            LogError()()("Not saving object with older revision.").Flush();
        } else {
            existing =
                std::make_shared<proto::BlockchainEthereumAccountData>(data);
        }
    }

    ethereum_account_types_[type].insert(accountID);
    ethereum_account_index_.emplace(accountID, type);
    ethereumLock.unlock();

    return save(writeLock);
}

auto Nym::Store(const UnitType type, const proto::HDAccount& data) -> bool
{
    const auto& accountID =
        factory_.AccountIDFromBase58(data.deterministic().common().id());

    if (accountID.empty()) {
        LogError()()("Invalid account ID.").Flush();

        return false;
    }

    if (false == proto::Validate(data, VERBOSE)) {
        LogError()()("Invalid account.").Flush();

        return false;
    }

    auto writeLock = Lock{write_lock_, std::defer_lock};
    auto blockchainLock = Lock{blockchain_lock_, std::defer_lock};
    std::lock(writeLock, blockchainLock);
    auto accountItem = blockchain_accounts_.find(accountID);

    if (blockchain_accounts_.end() == accountItem) {
        blockchain_accounts_[accountID] =
            std::make_shared<proto::HDAccount>(data);
    } else {
        auto& existing = accountItem->second;

        if (existing->deterministic().common().revision() >
            data.deterministic().common().revision()) {
            LogError()()("Not saving object with older revision.").Flush();
        } else {
            existing = std::make_shared<proto::HDAccount>(data);
        }
    }

    blockchain_account_types_[type].insert(accountID);
    blockchain_account_index_.emplace(accountID, type);
    blockchainLock.unlock();

    return save(writeLock);
}

auto Nym::Store(
    const proto::Nym& data,
    std::string_view alias,
    UnallocatedCString& plaintext) -> bool
{
    const auto lock = Lock{write_lock_};

    const std::uint64_t revision = data.revision();
    bool saveOk = false;
    const bool incomingPublic = (proto::NYM_PUBLIC == data.mode());
    const bool existing = is_valid(credentials_);

    if (existing) {
        if (incomingPublic) {
            if (checked_.get()) {
                saveOk = !private_.get();
            } else {
                std::shared_ptr<proto::Nym> serialized;
                using enum ErrorReporting;
                LoadProto(credentials_, serialized, silent);
                saveOk = !private_.get();
            }
        } else {
            saveOk = true;
        }
    } else {
        saveOk = true;
    }

    const bool keyUpgrade = (!incomingPublic) && (!private_.get());
    const bool revisionUpgrade = revision > revision_.load();
    const bool upgrade = keyUpgrade || revisionUpgrade;

    if (saveOk) {
        if (upgrade) {
            const auto saved =
                StoreProto<proto::Nym>(data, credentials_, plaintext);

            if (!saved) { return false; }

            revision_.store(revision);

            if (!alias.empty()) { alias_ = alias; }
        }
    }

    checked_->On();
    private_->Set(!incomingPublic);

    return save(lock);
}

auto Nym::Store(const proto::Purse& purse) -> bool
{
    const auto lock = Lock{write_lock_};
    const PurseID id{
        factory_.NotaryIDFromBase58(purse.notary()),
        factory_.UnitIDFromBase58(purse.mint())};
    auto hash = Hash{};
    const auto output = StoreProto(purse, hash);

    if (false == output) { return output; }

    purse_id_[id] = hash;

    return output;
}

auto Nym::threads() const -> tree::Threads*
{
    return construct<tree::Threads>(
        threads_lock_, threads_, threads_root_, *mail_inbox(), *mail_outbox());
}

auto Nym::Threads() const -> const tree::Threads& { return *threads(); }

auto Nym::workflows() const -> tree::PaymentWorkflows*
{
    return construct<tree::PaymentWorkflows>(
        workflows_lock_, workflows_, workflows_root_);
}

auto Nym::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        case 2u:
        case 3u:
        case 4u:
        case 5u:
        case 6u:
        case 7u:
        case 8u:
        case 9u:
        default: {
        }
    }

    if (is_valid(bip47_root_)) {
        if (auto* node = bip47(); node->Upgrade()) {
            bip47_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(sent_peer_request_)) {
        if (auto* node = sent_request_box(); node->Upgrade()) {
            sent_peer_request_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(incoming_peer_request_)) {
        if (auto* node = incoming_request_box(); node->Upgrade()) {
            incoming_peer_request_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(sent_peer_reply_)) {
        if (auto* node = sent_reply_box(); node->Upgrade()) {
            sent_peer_reply_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(incoming_peer_reply_)) {
        if (auto* node = incoming_reply_box(); node->Upgrade()) {
            incoming_peer_reply_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(finished_peer_request_)) {
        if (auto* node = finished_request_box(); node->Upgrade()) {
            finished_peer_request_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(finished_peer_reply_)) {
        if (auto* node = finished_reply_box(); node->Upgrade()) {
            finished_peer_reply_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(processed_peer_request_)) {
        if (auto* node = processed_request_box(); node->Upgrade()) {
            processed_peer_request_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(processed_peer_reply_)) {
        if (auto* node = processed_reply_box(); node->Upgrade()) {
            processed_peer_reply_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(mail_inbox_root_)) {
        if (auto* node = mail_inbox(); node->Upgrade()) {
            mail_inbox_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(mail_outbox_root_)) {
        if (auto* node = mail_outbox(); node->Upgrade()) {
            mail_outbox_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(threads_root_)) {
        if (auto* node = threads(); node->Upgrade()) {
            threads_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(contexts_root_)) {
        if (auto* node = contexts(); node->Upgrade()) {
            contexts_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(issuers_root_)) {
        if (auto* node = issuers(); node->Upgrade()) {
            issuers_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(workflows_root_)) {
        if (auto* node = workflows(); node->Upgrade()) {
            workflows_root_ = node->root_;
            changed = true;
        }
    }

    return changed;
}

Nym::~Nym() = default;
}  // namespace opentxs::storage::tree
