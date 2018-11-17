// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Nym.hpp"

#include "storage/Plugin.hpp"
#include "Bip47Channels.hpp"
#include "Contexts.hpp"
#include "Issuers.hpp"
#include "Mailbox.hpp"
#include "PaymentWorkflows.hpp"
#include "PeerReplies.hpp"
#include "PeerRequests.hpp"
#include "Thread.hpp"
#include "Threads.hpp"

#include <functional>

#define CURRENT_VERSION 7
#define BLOCKCHAIN_INDEX_VERSION 1

//#define OT_METHOD "opentxs::storage::Nym::"

namespace opentxs::storage
{
Nym::Nym(
    const opentxs::api::storage::Driver& storage,
    const std::string& id,
    const std::string& hash,
    const std::string& alias)
    : Node(storage, hash)
    , alias_(alias)
    , nymid_(id)
    , credentials_(Node::BLANK_HASH)
    , checked_(Flag::Factory(false))
    , private_(Flag::Factory(false))
    , revision_(0)
    , bip47_lock_{}
    , bip47_{nullptr}
    , bip47_root_{Node::BLANK_HASH}
    , sent_request_box_lock_()
    , sent_request_box_(nullptr)
    , sent_peer_request_(Node::BLANK_HASH)
    , incoming_request_box_lock_()
    , incoming_request_box_(nullptr)
    , incoming_peer_request_(Node::BLANK_HASH)
    , sent_reply_box_lock_()
    , sent_reply_box_(nullptr)
    , sent_peer_reply_(Node::BLANK_HASH)
    , incoming_reply_box_lock_()
    , incoming_reply_box_(nullptr)
    , incoming_peer_reply_(Node::BLANK_HASH)
    , finished_request_box_lock_()
    , finished_request_box_(nullptr)
    , finished_peer_request_(Node::BLANK_HASH)
    , finished_reply_box_lock_()
    , finished_reply_box_(nullptr)
    , finished_peer_reply_(Node::BLANK_HASH)
    , processed_request_box_lock_()
    , processed_request_box_(nullptr)
    , processed_peer_request_(Node::BLANK_HASH)
    , processed_reply_box_lock_()
    , processed_reply_box_(nullptr)
    , processed_peer_reply_(Node::BLANK_HASH)
    , mail_inbox_lock_()
    , mail_inbox_(nullptr)
    , mail_inbox_root_(Node::BLANK_HASH)
    , mail_outbox_lock_()
    , mail_outbox_(nullptr)
    , mail_outbox_root_(Node::BLANK_HASH)
    , threads_lock_()
    , threads_(nullptr)
    , threads_root_(Node::BLANK_HASH)
    , contexts_lock_()
    , contexts_(nullptr)
    , contexts_root_(Node::BLANK_HASH)
    , blockchain_lock_()
    , blockchain_account_types_()
    , blockchain_accounts_()
    , issuers_root_(Node::BLANK_HASH)
    , issuers_lock_()
    , issuers_(nullptr)
    , workflows_root_(Node::BLANK_HASH)
    , workflows_lock_()
    , workflows_(nullptr)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

std::string Nym::Alias() const { return alias_; }

storage::Bip47Channels* Nym::bip47() const
{
    return construct<storage::Bip47Channels>(bip47_lock_, bip47_, bip47_root_);
}

const storage::Bip47Channels& Nym::Bip47Channels() const { return *bip47(); }

std::set<std::string> Nym::BlockchainAccountList(
    const proto::ContactItemType type) const
{
    Lock lock(blockchain_lock_);

    auto it = blockchain_account_types_.find(type);

    if (blockchain_account_types_.end() == it) { return {}; }

    return it->second;
}

template <typename T, typename... Args>
T* Nym::construct(
    std::mutex& mutex,
    std::unique_ptr<T>& pointer,
    const std::string& root,
    Args&&... params) const
{
    Lock lock(mutex);

    if (false == bool(pointer)) {
        pointer.reset(new T(driver_, root, params...));

        if (!pointer) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();
            OT_FAIL;
        }
    }

    lock.unlock();

    return pointer.get();
}

storage::Contexts* Nym::contexts() const
{
    return construct<storage::Contexts>(
        contexts_lock_, contexts_, contexts_root_);
}

const storage::Contexts& Nym::Contexts() const { return *contexts(); }

template <typename T>
Editor<T> Nym::editor(
    std::string& root,
    std::mutex& mutex,
    T* (Nym::*get)() const)
{
    std::function<void(T*, Lock&)> callback = [&](T* in, Lock& lock) -> void {
        this->_save(in, lock, mutex, root);
    };

    return Editor<T>(write_lock_, (this->*get)(), callback);
}

PeerReplies* Nym::finished_reply_box() const
{
    return construct<storage::PeerReplies>(
        finished_reply_box_lock_, finished_reply_box_, finished_peer_reply_);
}

PeerRequests* Nym::finished_request_box() const
{
    return construct<storage::PeerRequests>(
        finished_request_box_lock_,
        finished_request_box_,
        finished_peer_request_);
}

const PeerRequests& Nym::FinishedRequestBox() const
{
    return *finished_request_box();
}

const PeerReplies& Nym::FinishedReplyBox() const
{
    return *finished_reply_box();
}

PeerReplies* Nym::incoming_reply_box() const
{
    return construct<storage::PeerReplies>(
        incoming_reply_box_lock_, incoming_reply_box_, incoming_peer_reply_);
}

PeerRequests* Nym::incoming_request_box() const
{
    return construct<storage::PeerRequests>(
        incoming_request_box_lock_,
        incoming_request_box_,
        incoming_peer_request_);
}

const PeerRequests& Nym::IncomingRequestBox() const
{
    return *incoming_request_box();
}

const PeerReplies& Nym::IncomingReplyBox() const
{
    return *incoming_reply_box();
}

void Nym::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNym> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load nym index file.")
            .Flush();
        OT_FAIL;
    }

    version_ = serialized->version();

    // Upgrade version
    if (CURRENT_VERSION > version_) { version_ = CURRENT_VERSION; }

    nymid_ = serialized->nymid();
    credentials_ = normalize_hash(serialized->credlist().hash());
    sent_peer_request_ = normalize_hash(serialized->sentpeerrequests().hash());
    incoming_peer_request_ =
        normalize_hash(serialized->incomingpeerrequests().hash());
    sent_peer_reply_ = normalize_hash(serialized->sentpeerreply().hash());
    incoming_peer_reply_ =
        normalize_hash(serialized->incomingpeerreply().hash());
    finished_peer_request_ =
        normalize_hash(serialized->finishedpeerrequest().hash());
    finished_peer_reply_ =
        normalize_hash(serialized->finishedpeerreply().hash());
    processed_peer_request_ =
        normalize_hash(serialized->processedpeerrequest().hash());
    processed_peer_reply_ =
        normalize_hash(serialized->processedpeerreply().hash());

    // Fields added in version 2
    if (serialized->has_mailinbox()) {
        mail_inbox_root_ = normalize_hash(serialized->mailinbox().hash());
    } else {
        mail_inbox_root_ = Node::BLANK_HASH;
    }

    if (serialized->has_mailoutbox()) {
        mail_outbox_root_ = normalize_hash(serialized->mailoutbox().hash());
    } else {
        mail_outbox_root_ = Node::BLANK_HASH;
    }

    if (serialized->has_threads()) {
        threads_root_ = normalize_hash(serialized->threads().hash());
    } else {
        threads_root_ = Node::BLANK_HASH;
    }

    // Fields added in version 3
    if (serialized->has_contexts()) {
        contexts_root_ = normalize_hash(serialized->contexts().hash());
    } else {
        contexts_root_ = Node::BLANK_HASH;
    }

    // Fields added in version 4
    for (const auto& it : serialized->blockchainaccountindex()) {
        const auto& id = it.id();
        auto& accountSet = blockchain_account_types_[id];

        for (const auto& accountID : it.list()) {
            accountSet.emplace(accountID);
        }
    }

    for (const auto& account : serialized->blockchainaccount()) {
        const auto& id = account.id();
        blockchain_accounts_.emplace(
            id, std::make_shared<proto::Bip44Account>(account));
    }

    // Fields added in version 5
    issuers_root_ = normalize_hash(serialized->issuers());

    // Fields added in version 6
    workflows_root_ = normalize_hash(serialized->paymentworkflow());

    // Fields added in version 7
    bip47_root_ = normalize_hash(serialized->bip47());
}

storage::Issuers* Nym::issuers() const
{
    return construct<storage::Issuers>(issuers_lock_, issuers_, issuers_root_);
}

const storage::Issuers& Nym::Issuers() const { return *issuers(); }

bool Nym::Load(
    const std::string& id,
    std::shared_ptr<proto::Bip44Account>& output,
    const bool checking) const
{
    Lock lock(blockchain_lock_);

    const auto it = blockchain_accounts_.find(id);

    if (blockchain_accounts_.end() == it) {
        if (false == checking) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Account does not exist.")
                .Flush();
        }

        return false;
    }

    output = it->second;

    return bool(output);
}

bool Nym::Load(
    std::shared_ptr<proto::CredentialIndex>& output,
    std::string& alias,
    const bool checking) const
{
    Lock lock(write_lock_);

    if (!check_hash(credentials_)) {
        if (!checking) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error: nym with id ")(nymid_)(
                " has no credentials.")
                .Flush();
        }

        return false;
    }

    alias = alias_;
    checked_->Set(driver_.LoadProto(credentials_, output, false));

    if (!checked_.get()) { return false; }

    private_->Set(proto::CREDINDEX_PRIVATE == output->mode());
    revision_.store(output->revision());

    return true;
}

Mailbox* Nym::mail_inbox() const
{
    return construct<storage::Mailbox>(
        mail_inbox_lock_, mail_inbox_, mail_inbox_root_);
}

Mailbox* Nym::mail_outbox() const
{
    return construct<storage::Mailbox>(
        mail_outbox_lock_, mail_outbox_, mail_outbox_root_);
}

const Mailbox& Nym::MailInbox() const { return *mail_inbox(); }

const Mailbox& Nym::MailOutbox() const { return *mail_outbox(); }

bool Nym::Migrate(const opentxs::api::storage::Driver& to) const
{
    bool output{true};
    output &= migrate(credentials_, to);
    output &= sent_request_box()->Migrate(to);
    output &= incoming_request_box()->Migrate(to);
    output &= sent_reply_box()->Migrate(to);
    output &= incoming_reply_box()->Migrate(to);
    output &= finished_request_box()->Migrate(to);
    output &= finished_reply_box()->Migrate(to);
    output &= processed_request_box()->Migrate(to);
    output &= processed_reply_box()->Migrate(to);
    output &= mail_inbox()->Migrate(to);
    output &= mail_outbox()->Migrate(to);
    output &= threads()->Migrate(to);
    output &= contexts()->Migrate(to);
    output &= issuers()->Migrate(to);
    output &= workflows()->Migrate(to);
    output &= bip47()->Migrate(to);
    output &= migrate(root_, to);

    return output;
}

Editor<storage::Bip47Channels> Nym::mutable_Bip47Channels()
{
    return editor<storage::Bip47Channels>(
        bip47_root_, bip47_lock_, &Nym::bip47);
}

Editor<PeerRequests> Nym::mutable_SentRequestBox()
{
    return editor<storage::PeerRequests>(
        sent_peer_request_, sent_request_box_lock_, &Nym::sent_request_box);
}

Editor<PeerRequests> Nym::mutable_IncomingRequestBox()
{
    return editor<storage::PeerRequests>(
        incoming_peer_request_,
        incoming_request_box_lock_,
        &Nym::incoming_request_box);
}

Editor<PeerReplies> Nym::mutable_SentReplyBox()
{
    return editor<storage::PeerReplies>(
        sent_peer_reply_, sent_reply_box_lock_, &Nym::sent_reply_box);
}

Editor<PeerReplies> Nym::mutable_IncomingReplyBox()
{
    return editor<storage::PeerReplies>(
        incoming_peer_reply_,
        incoming_reply_box_lock_,
        &Nym::incoming_reply_box);
}

Editor<PeerRequests> Nym::mutable_FinishedRequestBox()
{
    return editor<storage::PeerRequests>(
        finished_peer_request_,
        finished_request_box_lock_,
        &Nym::finished_request_box);
}

Editor<PeerReplies> Nym::mutable_FinishedReplyBox()
{
    return editor<storage::PeerReplies>(
        finished_peer_reply_,
        finished_reply_box_lock_,
        &Nym::finished_reply_box);
}

Editor<PeerRequests> Nym::mutable_ProcessedRequestBox()
{
    return editor<storage::PeerRequests>(
        processed_peer_request_,
        processed_request_box_lock_,
        &Nym::processed_request_box);
}

Editor<PeerReplies> Nym::mutable_ProcessedReplyBox()
{
    return editor<storage::PeerReplies>(
        processed_peer_reply_,
        processed_reply_box_lock_,
        &Nym::processed_reply_box);
}

Editor<Mailbox> Nym::mutable_MailInbox()
{
    return editor<storage::Mailbox>(
        mail_inbox_root_, mail_inbox_lock_, &Nym::mail_inbox);
}

Editor<Mailbox> Nym::mutable_MailOutbox()
{
    return editor<storage::Mailbox>(
        mail_outbox_root_, mail_outbox_lock_, &Nym::mail_outbox);
}

Editor<storage::Threads> Nym::mutable_Threads()
{
    return editor<storage::Threads>(
        threads_root_, threads_lock_, &Nym::threads);
}

Editor<storage::Contexts> Nym::mutable_Contexts()
{
    return editor<storage::Contexts>(
        contexts_root_, contexts_lock_, &Nym::contexts);
}

Editor<storage::Issuers> Nym::mutable_Issuers()
{
    return editor<storage::Issuers>(
        issuers_root_, issuers_lock_, &Nym::issuers);
}

Editor<storage::PaymentWorkflows> Nym::mutable_PaymentWorkflows()
{
    return editor<storage::PaymentWorkflows>(
        workflows_root_, workflows_lock_, &Nym::workflows);
}

const storage::PaymentWorkflows& Nym::PaymentWorkflows() const
{
    return *workflows();
}

PeerReplies* Nym::processed_reply_box() const
{
    return construct<storage::PeerReplies>(
        processed_reply_box_lock_, processed_reply_box_, processed_peer_reply_);
}

PeerRequests* Nym::processed_request_box() const
{
    return construct<storage::PeerRequests>(
        processed_request_box_lock_,
        processed_request_box_,
        processed_peer_request_);
}

const PeerRequests& Nym::ProcessedRequestBox() const
{
    return *processed_request_box();
}

const PeerReplies& Nym::ProcessedReplyBox() const
{
    return *processed_reply_box();
}

bool Nym::save(const Lock& lock) const
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL;
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

template <typename O>
void Nym::_save(
    O* input,
    const Lock& lock,
    std::mutex& mutex,
    std::string& root)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL;
    }

    if (nullptr == input) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL;
    }

    Lock rootLock(mutex);
    root = input->Root();
    rootLock.unlock();

    if (false == save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL;
    }
}

storage::Threads* Nym::threads() const
{
    return construct<storage::Threads>(
        threads_lock_, threads_, threads_root_, *mail_inbox(), *mail_outbox());
}

const storage::Threads& Nym::Threads() const { return *threads(); }

PeerReplies* Nym::sent_reply_box() const
{
    return construct<storage::PeerReplies>(
        sent_reply_box_lock_, sent_reply_box_, sent_peer_reply_);
}

PeerRequests* Nym::sent_request_box() const
{
    return construct<storage::PeerRequests>(
        sent_request_box_lock_, sent_request_box_, sent_peer_request_);
}

const PeerRequests& Nym::SentRequestBox() const { return *sent_request_box(); }

const PeerReplies& Nym::SentReplyBox() const { return *sent_reply_box(); }

proto::StorageNym Nym::serialize() const
{
    proto::StorageNym serialized;
    serialized.set_version(version_);
    serialized.set_nymid(nymid_);

    set_hash(version_, nymid_, credentials_, *serialized.mutable_credlist());
    set_hash(
        version_,
        nymid_,
        sent_peer_request_,
        *serialized.mutable_sentpeerrequests());
    set_hash(
        version_,
        nymid_,
        incoming_peer_request_,
        *serialized.mutable_incomingpeerrequests());
    set_hash(
        version_,
        nymid_,
        sent_peer_reply_,
        *serialized.mutable_sentpeerreply());
    set_hash(
        version_,
        nymid_,
        incoming_peer_reply_,
        *serialized.mutable_incomingpeerreply());
    set_hash(
        version_,
        nymid_,
        finished_peer_request_,
        *serialized.mutable_finishedpeerrequest());
    set_hash(
        version_,
        nymid_,
        finished_peer_reply_,
        *serialized.mutable_finishedpeerreply());
    set_hash(
        version_,
        nymid_,
        processed_peer_request_,
        *serialized.mutable_processedpeerrequest());
    set_hash(
        version_,
        nymid_,
        processed_peer_reply_,
        *serialized.mutable_processedpeerreply());
    set_hash(
        version_, nymid_, mail_inbox_root_, *serialized.mutable_mailinbox());
    set_hash(
        version_, nymid_, mail_outbox_root_, *serialized.mutable_mailoutbox());
    set_hash(version_, nymid_, threads_root_, *serialized.mutable_threads());
    set_hash(version_, nymid_, contexts_root_, *serialized.mutable_contexts());

    for (const auto& it : blockchain_account_types_) {
        const auto& chainType = it.first;
        const auto& accountSet = it.second;
        auto& index = *serialized.add_blockchainaccountindex();
        index.set_version(BLOCKCHAIN_INDEX_VERSION);
        index.set_id(chainType);

        for (const auto& accountID : accountSet) { index.add_list(accountID); }
    }

    for (const auto& it : blockchain_accounts_) {
        OT_ASSERT(it.second);

        const auto& account = *it.second;
        *serialized.add_blockchainaccount() = account;
    }

    serialized.set_issuers(issuers_root_);
    serialized.set_paymentworkflow(workflows_root_);
    serialized.set_bip47(bip47_root_);

    return serialized;
}

bool Nym::SetAlias(const std::string& alias)
{
    Lock lock(write_lock_);

    alias_ = alias;

    return true;
}

bool Nym::Store(
    const proto::ContactItemType type,
    const proto::Bip44Account& data)
{
    const auto& accountID = data.id();

    if (accountID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid account ID.").Flush();

        return false;
    }

    if (false == proto::Validate(data, VERBOSE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid account.").Flush();

        return false;
    }

    Lock writeLock(write_lock_, std::defer_lock);
    Lock blockchainLock(blockchain_lock_, std::defer_lock);
    std::lock(writeLock, blockchainLock);
    auto accountItem = blockchain_accounts_.find(accountID);

    if (blockchain_accounts_.end() == accountItem) {
        blockchain_accounts_[accountID] =
            std::make_shared<proto::Bip44Account>(data);
    } else {
        auto& existing = accountItem->second;

        if (existing->revision() > data.revision()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Not saving object with older revision.")
                .Flush();
        } else {
            existing = std::make_shared<proto::Bip44Account>(data);
        }
    }

    blockchain_account_types_[type].insert(accountID);
    blockchainLock.unlock();

    return save(writeLock);
}

bool Nym::Store(
    const proto::CredentialIndex& data,
    const std::string& alias,
    std::string& plaintext)
{
    Lock lock(write_lock_);

    const std::uint64_t revision = data.revision();
    bool saveOk = false;
    const bool incomingPublic = (proto::CREDINDEX_PUBLIC == data.mode());
    const bool existing = check_hash(credentials_);

    if (existing) {
        if (incomingPublic) {
            if (checked_.get()) {
                saveOk = !private_.get();
            } else {
                std::shared_ptr<proto::CredentialIndex> serialized;
                driver_.LoadProto(credentials_, serialized, true);
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
            const bool saved = driver_.StoreProto<proto::CredentialIndex>(
                data, credentials_, plaintext);

            if (!saved) { return false; }

            revision_.store(revision);

            if (!alias.empty()) { alias_ = alias; }
        }
    }

    checked_->On();
    private_->Set(!incomingPublic);

    return save(lock);
}

storage::PaymentWorkflows* Nym::workflows() const
{
    return construct<storage::PaymentWorkflows>(
        workflows_lock_, workflows_, workflows_root_);
}

Nym::~Nym() {}
}  // namespace opentxs::storage
