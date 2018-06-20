/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Nym, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "stdafx.hpp"

#include "Nym.hpp"

#include "storage/Plugin.hpp"
#include "Bip47Context.hpp"
#include "Bip47Contexts.hpp"
#include "Contexts.hpp"
#include "Issuers.hpp"
#include "Mailbox.hpp"
#include "PaymentWorkflows.hpp"
#include "PeerReplies.hpp"
#include "PeerRequests.hpp"
#include "Thread.hpp"
#include "Threads.hpp"

#include <functional>

#define CURRENT_VERSION 6
#define BLOCKCHAIN_INDEX_VERSION 1
#define BIP47_CHANNEL_INDEX_VERSION 1
#define OT_METHOD "opentxs::storage::Nym::"

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
    , bip47_lock_()
    , bip47_channels_()
    , bip47_contexts_lock_()
    , bip47_contexts_()
    , bip47_contexts_root_(Node::BLANK_HASH)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

std::string Nym::Alias() const { return alias_; }

std::set<proto::ContactItemType> Nym::Bip47ChainList() const
{
    std::set<proto::ContactItemType> output{};
    sLock lock(bip47_lock_);

    for (const auto& it : bip47_channels_) {
        if (!it.second.empty()) { output.emplace(it.first); }
    }

    return output;
}

std::set<Bip47ChannelID> Nym::Bip47ChannelList(
    const std::string& contactID,
    const proto::ContactItemType chain) const
{
    sLock lock(bip47_lock_);
    const auto outer = bip47_channels_.find(chain);

    // check: given chain exists
    if (outer == bip47_channels_.end()) { return {}; }

    const auto& innerMap = outer->second;

    // check: contact has channels
    const auto inner = innerMap.find(contactID);

    if (inner == innerMap.end()) { return {}; }

    return inner->second;
}

std::set<std::string> Nym::Bip47ContactList(
    const proto::ContactItemType chain) const
{
    std::set<std::string> contacts{};
    sLock lock(bip47_lock_);
    const auto it = bip47_channels_.find(chain);

    // check: given chain exists
    if (it == bip47_channels_.end()) { return contacts; }

    const auto& innerMap = it->second;

    for (const auto& channelID : innerMap) {
        // check: contact has at least one channel
        if (!channelID.second.empty()) { contacts.emplace(channelID.first); }
    }

    lock.unlock();
    return contacts;
}

class Contexts* Nym::contexts() const
{
    Lock lock(contexts_lock_);

    if (!contexts_) {
        contexts_.reset(new class Contexts(driver_, contexts_root_));

        if (!contexts_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return contexts_.get();
}

std::set<std::string> Nym::BlockchainAccountList(
    const proto::ContactItemType type) const
{
    Lock lock(blockchain_lock_);

    auto it = blockchain_account_types_.find(type);

    if (blockchain_account_types_.end() == it) { return {}; }

    return it->second;
}

const class Contexts& Nym::Contexts() const { return *contexts(); }

PeerReplies* Nym::finished_reply_box() const
{
    Lock lock(finished_reply_box_lock_);

    if (!finished_reply_box_) {
        finished_reply_box_.reset(
            new PeerReplies(driver_, finished_peer_reply_));

        if (!finished_reply_box_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return finished_reply_box_.get();
}

PeerRequests* Nym::finished_request_box() const
{
    Lock lock(finished_request_box_lock_);

    if (!finished_request_box_) {
        finished_request_box_.reset(
            new PeerRequests(driver_, finished_peer_request_));

        if (!finished_request_box_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return finished_request_box_.get();
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
    Lock lock(incoming_reply_box_lock_);

    if (!incoming_reply_box_) {
        incoming_reply_box_.reset(
            new PeerReplies(driver_, incoming_peer_reply_));

        if (!incoming_reply_box_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return incoming_reply_box_.get();
}

PeerRequests* Nym::incoming_request_box() const
{
    Lock lock(incoming_request_box_lock_);

    if (!incoming_request_box_) {
        incoming_request_box_.reset(
            new PeerRequests(driver_, incoming_peer_request_));

        if (!incoming_request_box_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return incoming_request_box_.get();
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
        otErr << __FUNCTION__ << ": Failed to load nym index file."
              << std::endl;
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
    for (const auto& it_channel : serialized->bip47channelindex()) {
        auto& chain = bip47_channels_[it_channel.chain()];
        auto& channels = chain[it_channel.contact()];

        for (const auto& pcode : it_channel.paymentcode()) {
            channels.emplace(Bip47ChannelID{it_channel.contact(), pcode});
        }
    }

    if (serialized->has_bip47contexts()) {
        threads_root_ = normalize_hash(serialized->bip47contexts().hash());
    } else {
        threads_root_ = Node::BLANK_HASH;
    }
}

class Issuers* Nym::issuers() const
{
    Lock lock(issuers_lock_);

    if (false == bool(issuers_)) {
        issuers_.reset(new class Issuers(driver_, issuers_root_));

        if (false == bool(issuers_)) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;

            OT_FAIL
        }
    }

    lock.unlock();

    return issuers_.get();
}

const class Issuers& Nym::Issuers() const { return *issuers(); }

bool Nym::Load(
    const std::string& id,
    std::shared_ptr<proto::Bip44Account>& output,
    const bool checking) const
{
    Lock lock(blockchain_lock_);

    const auto it = blockchain_accounts_.find(id);

    if (blockchain_accounts_.end() == it) {
        if (false == checking) {
            otErr << OT_METHOD << __FUNCTION__ << ": Account does not exist."
                  << std::endl;
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
    std::lock_guard<std::mutex> lock(write_lock_);

    if (!check_hash(credentials_)) {
        if (!checking) {
            otErr << __FUNCTION__ << ": Error: nym with id " << nymid_
                  << " has no credentials." << std::endl;
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
    Lock lock(mail_inbox_lock_);

    if (!mail_inbox_) {
        mail_inbox_.reset(new Mailbox(driver_, mail_inbox_root_));

        if (!mail_inbox_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return mail_inbox_.get();
}

Mailbox* Nym::mail_outbox() const
{
    Lock lock(mail_outbox_lock_);

    if (!mail_outbox_) {
        mail_outbox_.reset(new Mailbox(driver_, mail_outbox_root_));

        if (!mail_outbox_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return mail_outbox_.get();
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
    output &= migrate(root_, to);

    return output;
}

Editor<PeerRequests> Nym::mutable_SentRequestBox()
{
    std::function<void(PeerRequests*, Lock&)> callback =
        [&](PeerRequests* in, Lock& lock) -> void {
        this->save(in, lock, StorageBox::SENTPEERREQUEST);
    };

    return Editor<PeerRequests>(write_lock_, sent_request_box(), callback);
}

Editor<PeerRequests> Nym::mutable_IncomingRequestBox()
{
    std::function<void(PeerRequests*, Lock&)> callback =
        [&](PeerRequests* in, Lock& lock) -> void {
        this->save(in, lock, StorageBox::INCOMINGPEERREQUEST);
    };

    return Editor<PeerRequests>(write_lock_, incoming_request_box(), callback);
}

Editor<PeerReplies> Nym::mutable_SentReplyBox()
{
    std::function<void(PeerReplies*, Lock&)> callback =
        [&](PeerReplies* in, Lock& lock) -> void {
        this->save(in, lock, StorageBox::SENTPEERREPLY);
    };

    return Editor<PeerReplies>(write_lock_, sent_reply_box(), callback);
}

Editor<PeerReplies> Nym::mutable_IncomingReplyBox()
{
    std::function<void(PeerReplies*, Lock&)> callback =
        [&](PeerReplies* in, Lock& lock) -> void {
        this->save(in, lock, StorageBox::INCOMINGPEERREPLY);
    };

    return Editor<PeerReplies>(write_lock_, incoming_reply_box(), callback);
}

Editor<PeerRequests> Nym::mutable_FinishedRequestBox()
{
    std::function<void(PeerRequests*, Lock&)> callback =
        [&](PeerRequests* in, Lock& lock) -> void {
        this->save(in, lock, StorageBox::FINISHEDPEERREQUEST);
    };

    return Editor<PeerRequests>(write_lock_, finished_request_box(), callback);
}

Editor<PeerReplies> Nym::mutable_FinishedReplyBox()
{
    std::function<void(PeerReplies*, Lock&)> callback =
        [&](PeerReplies* in, Lock& lock) -> void {
        this->save(in, lock, StorageBox::FINISHEDPEERREPLY);
    };

    return Editor<PeerReplies>(write_lock_, finished_reply_box(), callback);
}

Editor<PeerRequests> Nym::mutable_ProcessedRequestBox()
{
    std::function<void(PeerRequests*, Lock&)> callback =
        [&](PeerRequests* in, Lock& lock) -> void {
        this->save(in, lock, StorageBox::PROCESSEDPEERREQUEST);
    };

    return Editor<PeerRequests>(write_lock_, processed_request_box(), callback);
}

Editor<PeerReplies> Nym::mutable_ProcessedReplyBox()
{
    std::function<void(PeerReplies*, Lock&)> callback =
        [&](PeerReplies* in, Lock& lock) -> void {
        this->save(in, lock, StorageBox::PROCESSEDPEERREPLY);
    };

    return Editor<PeerReplies>(write_lock_, processed_reply_box(), callback);
}

Editor<Mailbox> Nym::mutable_MailInbox()
{
    std::function<void(Mailbox*, Lock&)> callback = [&](Mailbox* in,
                                                        Lock& lock) -> void {
        this->save(in, lock, StorageBox::MAILINBOX);
    };

    return Editor<Mailbox>(write_lock_, mail_inbox(), callback);
}

Editor<Mailbox> Nym::mutable_MailOutbox()
{
    std::function<void(Mailbox*, Lock&)> callback = [&](Mailbox* in,
                                                        Lock& lock) -> void {
        this->save(in, lock, StorageBox::MAILOUTBOX);
    };

    return Editor<Mailbox>(write_lock_, mail_outbox(), callback);
}

Editor<class Threads> Nym::mutable_Threads()
{
    std::function<void(class Threads*, Lock&)> callback =
        [&](class Threads* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<class Threads>(write_lock_, threads(), callback);
}

Editor<class Contexts> Nym::mutable_Contexts()
{
    std::function<void(class Contexts*, Lock&)> callback =
        [&](class Contexts* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<class Contexts>(write_lock_, contexts(), callback);
}

Editor<class Issuers> Nym::mutable_Issuers()
{
    std::function<void(class Issuers*, Lock&)> callback =
        [&](class Issuers* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<class Issuers>(write_lock_, issuers(), callback);
}

Editor<class PaymentWorkflows> Nym::mutable_PaymentWorkflows()
{
    std::function<void(class PaymentWorkflows*, Lock&)> callback =
        [&](class PaymentWorkflows* in, Lock& lock) -> void {
        this->save(in, lock);
    };

    return Editor<class PaymentWorkflows>(write_lock_, workflows(), callback);
}

Editor<class Bip47Contexts> Nym::mutable_Bip47Contexts()
{
    std::function<void(class Bip47Contexts*, Lock&)> callback =
        [&](class Bip47Contexts* in, Lock& lock) -> void {
        this->save(in, lock);
    };

    return Editor<class Bip47Contexts>(write_lock_, bip47contexts(), callback);
}

const class PaymentWorkflows& Nym::PaymentWorkflows() const
{
    return *workflows();
}

const class Bip47Contexts& Nym::Bip47Contexts() const
{
    return *bip47contexts();
}

PeerReplies* Nym::processed_reply_box() const
{
    Lock lock(processed_reply_box_lock_);

    if (!processed_reply_box_) {
        processed_reply_box_.reset(
            new PeerReplies(driver_, processed_peer_reply_));

        if (!processed_reply_box_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return processed_reply_box_.get();
}

PeerRequests* Nym::processed_request_box() const
{
    Lock lock(processed_request_box_lock_);

    if (!processed_request_box_) {
        processed_request_box_.reset(
            new PeerRequests(driver_, processed_peer_request_));

        if (!processed_request_box_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return processed_request_box_.get();
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
        otErr << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

void Nym::save(PeerReplies* input, const Lock& lock, StorageBox type)
{
    if (!verify_write_lock(lock)) {
        otErr << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    if (nullptr == input) {
        otErr << __FUNCTION__ << ": Null target" << std::endl;
        OT_FAIL;
    }

    update_hash(type, input->Root());

    if (!save(lock)) {
        otErr << __FUNCTION__ << ": Save error" << std::endl;
        OT_FAIL;
    }
}

void Nym::save(PeerRequests* input, const Lock& lock, StorageBox type)
{
    if (!verify_write_lock(lock)) {
        otErr << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    if (nullptr == input) {
        otErr << __FUNCTION__ << ": Null target" << std::endl;
        OT_FAIL;
    }

    update_hash(type, input->Root());

    if (!save(lock)) {
        otErr << __FUNCTION__ << ": Save error" << std::endl;
        OT_FAIL;
    }
}

void Nym::save(Mailbox* input, const Lock& lock, StorageBox type)
{
    if (!verify_write_lock(lock)) {
        otErr << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    if (nullptr == input) {
        otErr << __FUNCTION__ << ": Null target" << std::endl;
        OT_FAIL;
    }

    update_hash(type, input->Root());

    if (!save(lock)) {
        otErr << __FUNCTION__ << ": Save error" << std::endl;
        OT_FAIL;
    }
}

void Nym::save(class Threads* input, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    if (nullptr == input) {
        otErr << __FUNCTION__ << ": Null target" << std::endl;
        OT_FAIL;
    }

    if (mail_inbox_) {
        update_hash(StorageBox::MAILINBOX, mail_inbox_->Root());
    }

    if (mail_outbox_) {
        update_hash(StorageBox::MAILOUTBOX, mail_outbox_->Root());
    }

    threads_root_ = input->Root();

    if (!save(lock)) {
        otErr << __FUNCTION__ << ": Save error" << std::endl;
        OT_FAIL;
    }
}

void Nym::save(class Contexts* input, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    if (nullptr == input) {
        otErr << __FUNCTION__ << ": Null target" << std::endl;
        OT_FAIL;
    }

    contexts_root_ = input->Root();

    if (!save(lock)) {
        otErr << __FUNCTION__ << ": Save error" << std::endl;
        OT_FAIL;
    }
}

void Nym::save(class Issuers* input, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    if (nullptr == input) {
        otErr << __FUNCTION__ << ": Null target" << std::endl;
        OT_FAIL;
    }

    issuers_root_ = input->Root();

    if (!save(lock)) {
        otErr << __FUNCTION__ << ": Save error" << std::endl;
        OT_FAIL;
    }
}

void Nym::save(class PaymentWorkflows* input, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    if (nullptr == input) {
        otErr << __FUNCTION__ << ": Null target" << std::endl;
        OT_FAIL;
    }

    workflows_root_ = input->Root();

    if (!save(lock)) {
        otErr << __FUNCTION__ << ": Save error" << std::endl;
        OT_FAIL;
    }
}

void Nym::save(class Bip47Contexts* input, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        otErr << __FUNCTION__ << ": Lock failure." << std::endl;
        OT_FAIL;
    }

    if (nullptr == input) {
        otErr << __FUNCTION__ << ": Null target" << std::endl;
        OT_FAIL;
    }

    bip47_contexts_root_ = input->Root();

    if (!save(lock)) {
        otErr << __FUNCTION__ << ": Save error" << std::endl;
        OT_FAIL;
    }
}

class Threads* Nym::threads() const
{
    Lock lock(threads_lock_);

    if (!threads_) {
        threads_.reset(new class Threads(
            driver_, threads_root_, *mail_inbox(), *mail_outbox()));

        if (!threads_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return threads_.get();
}

const class Threads& Nym::Threads() const { return *threads(); }

void Nym::update_hash(const StorageBox type, const std::string& root)
{
    switch (type) {
        case StorageBox::SENTPEERREQUEST: {
            std::lock_guard<std::mutex> lock(sent_request_box_lock_);
            sent_peer_request_ = root;
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            std::lock_guard<std::mutex> lock(incoming_request_box_lock_);
            incoming_peer_request_ = root;
        } break;
        case StorageBox::SENTPEERREPLY: {
            std::lock_guard<std::mutex> lock(sent_reply_box_lock_);
            sent_peer_reply_ = root;
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            std::lock_guard<std::mutex> lock(incoming_reply_box_lock_);
            incoming_peer_reply_ = root;
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            std::lock_guard<std::mutex> lock(finished_request_box_lock_);
            finished_peer_request_ = root;
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            std::lock_guard<std::mutex> lock(finished_reply_box_lock_);
            finished_peer_reply_ = root;
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            std::lock_guard<std::mutex> lock(finished_reply_box_lock_);
            processed_peer_request_ = root;
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            std::lock_guard<std::mutex> lock(processed_reply_box_lock_);
            processed_peer_reply_ = root;
        } break;
        case StorageBox::MAILINBOX: {
            std::lock_guard<std::mutex> lock(mail_inbox_lock_);
            mail_inbox_root_ = root;
        } break;
        case StorageBox::MAILOUTBOX: {
            std::lock_guard<std::mutex> lock(mail_outbox_lock_);
            mail_outbox_root_ = root;
        } break;
        default: {
            otErr << __FUNCTION__ << ": Unknown box" << std::endl;
            OT_FAIL;
        }
    }
}

PeerReplies* Nym::sent_reply_box() const
{
    Lock lock(sent_reply_box_lock_);

    if (!sent_reply_box_) {
        sent_reply_box_.reset(new PeerReplies(driver_, sent_peer_reply_));

        if (!sent_reply_box_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return sent_reply_box_.get();
}

PeerRequests* Nym::sent_request_box() const
{
    Lock lock(sent_request_box_lock_);

    if (!sent_request_box_) {
        sent_request_box_.reset(new PeerRequests(driver_, sent_peer_request_));

        if (!sent_request_box_) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;
            OT_FAIL;
        }
    }

    lock.unlock();

    return sent_request_box_.get();
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

    for (const auto& it_chain : bip47_channels_) {
        auto& chain = it_chain.first;

        for (const auto& it_contact : it_chain.second) {
            auto& channel = *serialized.add_bip47channelindex();
            channel.set_version(BIP47_CHANNEL_INDEX_VERSION);
            channel.set_chain(chain);
            channel.set_contact(it_contact.first);

            for (const auto& chanid : it_contact.second) {
                *channel.add_paymentcode() = chanid.second;
            }
        }
    }

    set_hash(
        version_,
        nymid_,
        bip47_contexts_root_,
        *serialized.mutable_bip47contexts());
    serialized.set_issuers(issuers_root_);
    serialized.set_paymentworkflow(workflows_root_);

    return serialized;
}

bool Nym::SetAlias(const std::string& alias)
{
    std::lock_guard<std::mutex> lock(write_lock_);

    alias_ = alias;

    return true;
}

bool Nym::Store(
    const proto::ContactItemType type,
    const proto::Bip44Account& data)
{
    const auto& accountID = data.id();

    if (accountID.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid account ID."
              << std::endl;

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
            otErr << OT_METHOD << __FUNCTION__
                  << ": Not saving object with older revision." << std::endl;
        } else {
            existing = std::make_shared<proto::Bip44Account>(data);
        }
    }

    blockchain_account_types_[type].insert(accountID);
    blockchainLock.unlock();

    return save(writeLock);
}

bool Nym::Store(const proto::Bip47Context& context)
{
    const auto& paymentCode = context.paymentcode();

    if (paymentCode.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid payment code."
              << std::endl;

        return false;
    }

    if (false == proto::Validate(context, VERBOSE)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid BIP47Context."
              << std::endl;

        return false;
    }

    Lock writeLock(write_lock_, std::defer_lock);
    sLock bip47Lock(bip47_lock_, std::defer_lock);
    std::lock(writeLock, bip47Lock);

    for (const auto& chain : context.chain()) {
        auto& chans = bip47_channels_[chain.type()];
        for (const auto& channel : chain.channel()) {
            auto& channels = chans[channel.contact()];
            channels.emplace(
                Bip47ChannelID{channel.contact(), channel.paymentcode()});
        }
    }

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

class PaymentWorkflows* Nym::workflows() const
{
    Lock lock(workflows_lock_);

    if (false == bool(workflows_)) {
        workflows_.reset(new class PaymentWorkflows(driver_, workflows_root_));

        if (false == bool(workflows_)) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;

            OT_FAIL
        }
    }

    lock.unlock();

    return workflows_.get();
}

class Bip47Contexts* Nym::bip47contexts() const
{
    Lock lock(bip47_contexts_lock_);

    if (false == bool(bip47_contexts_)) {
        bip47_contexts_.reset(
            new class Bip47Contexts(driver_, bip47_contexts_root_));

        if (false == bool(bip47_contexts_)) {
            otErr << __FUNCTION__ << ": Unable to instantiate." << std::endl;

            OT_FAIL
        }
    }

    lock.unlock();

    return bip47_contexts_.get();
}

Nym::~Nym() {}
}  // namespace opentxs::storage
