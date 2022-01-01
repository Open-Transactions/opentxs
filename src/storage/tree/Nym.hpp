// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "serialization/protobuf/StorageNym.pb.h"
#include "storage/tree/Node.hpp"

namespace opentxs
{
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

class Data;
class Identifier;
}  // namespace opentxs

namespace opentxs::storage
{
class Nym final : public Node
{
public:
    auto BlockchainAccountList(const core::UnitType type) const
        -> std::pmr::set<std::string>;
    auto BlockchainAccountType(const std::string& accountID) const
        -> core::UnitType;

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
        const Data& txid,
        const Identifier& contact,
        const bool add) -> Editor<storage::Threads>;
    auto mutable_PaymentWorkflows() -> Editor<storage::PaymentWorkflows>;

    auto Alias() const -> std::string;
    auto Load(
        const std::string& id,
        std::shared_ptr<proto::HDAccount>& output,
        const bool checking) const -> bool;
    auto Load(
        std::shared_ptr<proto::Nym>& output,
        std::string& alias,
        const bool checking) const -> bool;
    auto Load(
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        std::shared_ptr<proto::Purse>& output,
        const bool checking) const -> bool;
    auto Migrate(const Driver& to) const -> bool final;

    auto SetAlias(const std::string& alias) -> bool;
    auto Store(const core::UnitType type, const proto::HDAccount& data) -> bool;
    auto Store(
        const proto::Nym& data,
        const std::string& alias,
        std::string& plaintext) -> bool;
    auto Store(const proto::Purse& purse) -> bool;

    ~Nym() final;

private:
    friend Nyms;

    using PurseID = std::pair<OTServerID, OTUnitID>;

    std::string alias_;
    std::string nymid_;
    std::string credentials_;

    mutable OTFlag checked_;
    mutable OTFlag private_;
    mutable std::atomic<std::uint64_t> revision_;

    mutable std::mutex bip47_lock_;
    mutable std::unique_ptr<storage::Bip47Channels> bip47_;
    std::string bip47_root_;
    mutable std::mutex sent_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> sent_request_box_;
    std::string sent_peer_request_;
    mutable std::mutex incoming_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> incoming_request_box_;
    std::string incoming_peer_request_;
    mutable std::mutex sent_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> sent_reply_box_;
    std::string sent_peer_reply_;
    mutable std::mutex incoming_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> incoming_reply_box_;
    std::string incoming_peer_reply_;
    mutable std::mutex finished_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> finished_request_box_;
    std::string finished_peer_request_;
    mutable std::mutex finished_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> finished_reply_box_;
    std::string finished_peer_reply_;
    mutable std::mutex processed_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> processed_request_box_;
    std::string processed_peer_request_;
    mutable std::mutex processed_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> processed_reply_box_;
    std::string processed_peer_reply_;
    mutable std::mutex mail_inbox_lock_;
    mutable std::unique_ptr<Mailbox> mail_inbox_;
    std::string mail_inbox_root_;
    mutable std::mutex mail_outbox_lock_;
    mutable std::unique_ptr<Mailbox> mail_outbox_;
    std::string mail_outbox_root_;
    mutable std::mutex threads_lock_;
    mutable std::unique_ptr<storage::Threads> threads_;
    std::string threads_root_;
    mutable std::mutex contexts_lock_;
    mutable std::unique_ptr<storage::Contexts> contexts_;
    std::string contexts_root_;
    mutable std::mutex blockchain_lock_;
    std::pmr::map<core::UnitType, std::pmr::set<std::string>>
        blockchain_account_types_{};
    std::pmr::map<std::string, core::UnitType> blockchain_account_index_;
    std::pmr::map<std::string, std::shared_ptr<proto::HDAccount>>
        blockchain_accounts_{};
    std::string issuers_root_;
    mutable std::mutex issuers_lock_;
    mutable std::unique_ptr<storage::Issuers> issuers_;
    std::string workflows_root_;
    mutable std::mutex workflows_lock_;
    mutable std::unique_ptr<storage::PaymentWorkflows> workflows_;
    std::pmr::map<PurseID, std::string> purse_id_;

    template <typename T, typename... Args>
    auto construct(
        std::mutex& mutex,
        std::unique_ptr<T>& pointer,
        const std::string& root,
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
    auto editor(std::string& root, std::mutex& mutex, T* (Nym::*get)() const)
        -> Editor<T>;

    void init(const std::string& hash) final;
    auto save(const Lock& lock) const -> bool final;
    template <typename O>
    void _save(
        O* input,
        const Lock& lock,
        std::mutex& mutex,
        std::string& root);
    auto serialize() const -> proto::StorageNym;

    Nym(const Driver& storage,
        const std::string& id,
        const std::string& hash,
        const std::string& alias);
    Nym() = delete;
    Nym(const identity::Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const identity::Nym&) -> Nym = delete;
    auto operator=(Nym&&) -> Nym = delete;
};
}  // namespace opentxs::storage
