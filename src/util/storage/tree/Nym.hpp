// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageNym.pb.h>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

#include "internal/util/Editor.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace proto
{
class HDAccount;
class Nym;
class Purse;
}  // namespace proto

namespace storage
{
class Bip47Channels;
class Contexts;
class Driver;
class Issuers;
class Mailbox;
class Nyms;
class PaymentWorkflows;
class PeerReplies;
class PeerRequests;
class Threads;
}  // namespace storage

}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Nym final : public Node
{
public:
    auto BlockchainAccountList(const UnitType type) const
        -> UnallocatedSet<identifier::Account>;
    auto BlockchainAccountType(const identifier::Account& accountID) const
        -> UnitType;

    auto Bip47Channels() const -> const storage::Bip47Channels&;
    auto Contexts() const -> const storage::Contexts&;
    auto FinishedReplyBox() const -> const PeerReplies&;
    auto FinishedRequestBox() const -> const PeerRequests&;
    auto IncomingReplyBox() const -> const PeerReplies&;
    auto IncomingRequestBox() const -> const PeerRequests&;
    auto Issuers() const -> const storage::Issuers&;
    auto MailInbox() const -> const Mailbox&;
    auto MailOutbox() const -> const Mailbox&;
    auto ProcessedReplyBox() const -> const PeerReplies&;
    auto ProcessedRequestBox() const -> const PeerRequests&;
    auto SentReplyBox() const -> const PeerReplies&;
    auto SentRequestBox() const -> const PeerRequests&;
    auto Threads() const -> const storage::Threads&;
    auto PaymentWorkflows() const -> const storage::PaymentWorkflows&;

    auto mutable_Bip47Channels() -> Editor<storage::Bip47Channels>;
    auto mutable_Contexts() -> Editor<storage::Contexts>;
    auto mutable_FinishedReplyBox() -> Editor<PeerReplies>;
    auto mutable_FinishedRequestBox() -> Editor<PeerRequests>;
    auto mutable_IncomingReplyBox() -> Editor<PeerReplies>;
    auto mutable_IncomingRequestBox() -> Editor<PeerRequests>;
    auto mutable_Issuers() -> Editor<storage::Issuers>;
    auto mutable_MailInbox() -> Editor<Mailbox>;
    auto mutable_MailOutbox() -> Editor<Mailbox>;
    auto mutable_ProcessedReplyBox() -> Editor<PeerReplies>;
    auto mutable_ProcessedRequestBox() -> Editor<PeerRequests>;
    auto mutable_SentReplyBox() -> Editor<PeerReplies>;
    auto mutable_SentRequestBox() -> Editor<PeerRequests>;
    auto mutable_Threads() -> Editor<storage::Threads>;
    auto mutable_Threads(
        const blockchain::block::TransactionHash& txid,
        const identifier::Generic& contact,
        const bool add) -> Editor<storage::Threads>;
    auto mutable_PaymentWorkflows() -> Editor<storage::PaymentWorkflows>;

    auto Alias() const -> UnallocatedCString;
    auto Load(
        const identifier::Account& id,
        std::shared_ptr<proto::HDAccount>& output,
        const bool checking) const -> bool;
    auto Load(
        std::shared_ptr<proto::Nym>& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool;
    auto Load(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        std::shared_ptr<proto::Purse>& output,
        const bool checking) const -> bool;
    auto Migrate(const Driver& to) const -> bool final;

    auto SetAlias(const UnallocatedCString& alias) -> bool;
    auto Store(const UnitType type, const proto::HDAccount& data) -> bool;
    auto Store(
        const proto::Nym& data,
        const UnallocatedCString& alias,
        UnallocatedCString& plaintext) -> bool;
    auto Store(const proto::Purse& purse) -> bool;

    Nym(const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& id,
        const UnallocatedCString& hash,
        const UnallocatedCString& alias);
    Nym() = delete;
    Nym(const identity::Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const identity::Nym&) -> Nym = delete;
    auto operator=(Nym&&) -> Nym = delete;

    ~Nym() final;

private:
    friend Nyms;

    using PurseID = std::pair<identifier::Notary, identifier::UnitDefinition>;

    static constexpr auto current_version_ = VersionNumber{9};
    static constexpr auto blockchain_index_version_ = VersionNumber{1};
    static constexpr auto storage_purse_version_ = VersionNumber{1};

    UnallocatedCString alias_;
    UnallocatedCString nymid_;
    UnallocatedCString credentials_;

    mutable OTFlag checked_;
    mutable OTFlag private_;
    mutable std::atomic<std::uint64_t> revision_;

    mutable std::mutex bip47_lock_;
    mutable std::unique_ptr<storage::Bip47Channels> bip47_;
    UnallocatedCString bip47_root_;
    mutable std::mutex sent_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> sent_request_box_;
    UnallocatedCString sent_peer_request_;
    mutable std::mutex incoming_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> incoming_request_box_;
    UnallocatedCString incoming_peer_request_;
    mutable std::mutex sent_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> sent_reply_box_;
    UnallocatedCString sent_peer_reply_;
    mutable std::mutex incoming_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> incoming_reply_box_;
    UnallocatedCString incoming_peer_reply_;
    mutable std::mutex finished_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> finished_request_box_;
    UnallocatedCString finished_peer_request_;
    mutable std::mutex finished_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> finished_reply_box_;
    UnallocatedCString finished_peer_reply_;
    mutable std::mutex processed_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> processed_request_box_;
    UnallocatedCString processed_peer_request_;
    mutable std::mutex processed_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> processed_reply_box_;
    UnallocatedCString processed_peer_reply_;
    mutable std::mutex mail_inbox_lock_;
    mutable std::unique_ptr<Mailbox> mail_inbox_;
    UnallocatedCString mail_inbox_root_;
    mutable std::mutex mail_outbox_lock_;
    mutable std::unique_ptr<Mailbox> mail_outbox_;
    UnallocatedCString mail_outbox_root_;
    mutable std::mutex threads_lock_;
    mutable std::unique_ptr<storage::Threads> threads_;
    UnallocatedCString threads_root_;
    mutable std::mutex contexts_lock_;
    mutable std::unique_ptr<storage::Contexts> contexts_;
    UnallocatedCString contexts_root_;
    mutable std::mutex blockchain_lock_;
    UnallocatedMap<UnitType, UnallocatedSet<identifier::Account>>
        blockchain_account_types_{};
    UnallocatedMap<identifier::Account, UnitType> blockchain_account_index_;
    UnallocatedMap<identifier::Account, std::shared_ptr<proto::HDAccount>>
        blockchain_accounts_{};
    UnallocatedCString issuers_root_;
    mutable std::mutex issuers_lock_;
    mutable std::unique_ptr<storage::Issuers> issuers_;
    UnallocatedCString workflows_root_;
    mutable std::mutex workflows_lock_;
    mutable std::unique_ptr<storage::PaymentWorkflows> workflows_;
    UnallocatedMap<PurseID, UnallocatedCString> purse_id_;

    template <typename T, typename... Args>
    auto construct(
        std::mutex& mutex,
        std::unique_ptr<T>& pointer,
        const UnallocatedCString& root,
        Args&&... params) const -> T*;

    auto bip47() const -> storage::Bip47Channels*;
    auto sent_request_box() const -> PeerRequests*;
    auto incoming_request_box() const -> PeerRequests*;
    auto sent_reply_box() const -> PeerReplies*;
    auto incoming_reply_box() const -> PeerReplies*;
    auto finished_request_box() const -> PeerRequests*;
    auto finished_reply_box() const -> PeerReplies*;
    auto processed_request_box() const -> PeerRequests*;
    auto processed_reply_box() const -> PeerReplies*;
    auto mail_inbox() const -> Mailbox*;
    auto mail_outbox() const -> Mailbox*;
    auto threads() const -> storage::Threads*;
    auto contexts() const -> storage::Contexts*;
    auto issuers() const -> storage::Issuers*;
    auto workflows() const -> storage::PaymentWorkflows*;

    template <typename T>
    auto editor(
        UnallocatedCString& root,
        std::mutex& mutex,
        T* (Nym::*get)() const) -> Editor<T>;

    void init(const UnallocatedCString& hash) final;
    auto save(const Lock& lock) const -> bool final;
    template <typename O>
    void _save(
        O* input,
        const Lock& lock,
        std::mutex& mutex,
        UnallocatedCString& root);
    auto serialize() const -> proto::StorageNym;
};
}  // namespace opentxs::storage
