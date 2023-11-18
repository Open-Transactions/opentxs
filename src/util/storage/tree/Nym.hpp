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
#include <string_view>
#include <utility>

#include "internal/util/Editor.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/core/FixedByteArray.hpp"  // IWYU pragma: keep
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/storage/Types.hpp"
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
class BlockchainEthereumAccountData;
class HDAccount;
class Nym;
class Purse;
}  // namespace proto

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Bip47Channels;
class Contexts;
class Issuers;
class Mailbox;
class Nyms;
class PaymentWorkflows;
class PeerReplies;
class PeerRequests;
class Threads;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Nym final : public Node
{
public:
    auto BlockchainAccountList(const UnitType type) const
        -> UnallocatedSet<identifier::Account>;
    auto BlockchainAccountType(const identifier::Account& accountID) const
        -> UnitType;
    auto BlockchainEthereumAccountList(const UnitType type) const
        -> UnallocatedSet<identifier::Account>;
    auto BlockchainEthereumAccountType(
        const identifier::Account& accountID) const -> UnitType;

    auto Bip47Channels() const -> const tree::Bip47Channels&;
    auto Contexts() const -> const tree::Contexts&;
    auto FinishedReplyBox() const -> const PeerReplies&;
    auto FinishedRequestBox() const -> const PeerRequests&;
    auto IncomingReplyBox() const -> const PeerReplies&;
    auto IncomingRequestBox() const -> const PeerRequests&;
    auto Issuers() const -> const tree::Issuers&;
    auto MailInbox() const -> const Mailbox&;
    auto MailOutbox() const -> const Mailbox&;
    auto ProcessedReplyBox() const -> const PeerReplies&;
    auto ProcessedRequestBox() const -> const PeerRequests&;
    auto SentReplyBox() const -> const PeerReplies&;
    auto SentRequestBox() const -> const PeerRequests&;
    auto Threads() const -> const tree::Threads&;
    auto PaymentWorkflows() const -> const tree::PaymentWorkflows&;

    auto mutable_Bip47Channels() -> Editor<tree::Bip47Channels>;
    auto mutable_Contexts() -> Editor<tree::Contexts>;
    auto mutable_FinishedReplyBox() -> Editor<PeerReplies>;
    auto mutable_FinishedRequestBox() -> Editor<PeerRequests>;
    auto mutable_IncomingReplyBox() -> Editor<PeerReplies>;
    auto mutable_IncomingRequestBox() -> Editor<PeerRequests>;
    auto mutable_Issuers() -> Editor<tree::Issuers>;
    auto mutable_MailInbox() -> Editor<Mailbox>;
    auto mutable_MailOutbox() -> Editor<Mailbox>;
    auto mutable_ProcessedReplyBox() -> Editor<PeerReplies>;
    auto mutable_ProcessedRequestBox() -> Editor<PeerRequests>;
    auto mutable_SentReplyBox() -> Editor<PeerReplies>;
    auto mutable_SentRequestBox() -> Editor<PeerRequests>;
    auto mutable_Threads() -> Editor<tree::Threads>;
    auto mutable_Threads(
        const blockchain::block::TransactionHash& txid,
        const identifier::Generic& contact,
        const bool add) -> Editor<tree::Threads>;
    auto mutable_PaymentWorkflows() -> Editor<tree::PaymentWorkflows>;

    auto Alias() const -> UnallocatedCString;
    auto Load(
        const identifier::Account& id,
        std::shared_ptr<proto::BlockchainEthereumAccountData>& output,
        ErrorReporting checking) const -> bool;
    auto Load(
        const identifier::Account& id,
        std::shared_ptr<proto::HDAccount>& output,
        ErrorReporting checking) const -> bool;
    auto Load(
        std::shared_ptr<proto::Nym>& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const -> bool;
    auto Load(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        std::shared_ptr<proto::Purse>& output,
        ErrorReporting checking) const -> bool;

    auto SetAlias(std::string_view alias) -> bool;
    auto Store(
        const UnitType type,
        const proto::BlockchainEthereumAccountData& data) -> bool;
    auto Store(const UnitType type, const proto::HDAccount& data) -> bool;
    auto Store(
        const proto::Nym& data,
        std::string_view alias,
        UnallocatedCString& plaintext) -> bool;
    auto Store(const proto::Purse& purse) -> bool;

    Nym(const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const identifier::Nym& id,
        const Hash& hash,
        std::string_view alias);
    Nym() = delete;
    Nym(const identity::Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const identity::Nym&) -> Nym = delete;
    auto operator=(Nym&&) -> Nym = delete;

    ~Nym() final;

private:
    friend Nyms;

    using PurseID = std::pair<identifier::Notary, identifier::UnitDefinition>;

    static constexpr auto current_version_ = VersionNumber{10};
    static constexpr auto blockchain_index_version_ = VersionNumber{1};
    static constexpr auto storage_purse_version_ = VersionNumber{1};

    UnallocatedCString alias_;
    const identifier::Nym nymid_;
    Hash credentials_;

    mutable OTFlag checked_;
    mutable OTFlag private_;
    mutable std::atomic<std::uint64_t> revision_;

    mutable std::mutex bip47_lock_;
    mutable std::unique_ptr<tree::Bip47Channels> bip47_;
    Hash bip47_root_;
    mutable std::mutex sent_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> sent_request_box_;
    Hash sent_peer_request_;
    mutable std::mutex incoming_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> incoming_request_box_;
    Hash incoming_peer_request_;
    mutable std::mutex sent_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> sent_reply_box_;
    Hash sent_peer_reply_;
    mutable std::mutex incoming_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> incoming_reply_box_;
    Hash incoming_peer_reply_;
    mutable std::mutex finished_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> finished_request_box_;
    Hash finished_peer_request_;
    mutable std::mutex finished_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> finished_reply_box_;
    Hash finished_peer_reply_;
    mutable std::mutex processed_request_box_lock_;
    mutable std::unique_ptr<PeerRequests> processed_request_box_;
    Hash processed_peer_request_;
    mutable std::mutex processed_reply_box_lock_;
    mutable std::unique_ptr<PeerReplies> processed_reply_box_;
    Hash processed_peer_reply_;
    mutable std::mutex mail_inbox_lock_;
    mutable std::unique_ptr<Mailbox> mail_inbox_;
    Hash mail_inbox_root_;
    mutable std::mutex mail_outbox_lock_;
    mutable std::unique_ptr<Mailbox> mail_outbox_;
    Hash mail_outbox_root_;
    mutable std::mutex threads_lock_;
    mutable std::unique_ptr<tree::Threads> threads_;
    Hash threads_root_;
    mutable std::mutex contexts_lock_;
    mutable std::unique_ptr<tree::Contexts> contexts_;
    Hash contexts_root_;
    mutable std::mutex blockchain_lock_;
    UnallocatedMap<UnitType, UnallocatedSet<identifier::Account>>
        blockchain_account_types_{};
    UnallocatedMap<identifier::Account, UnitType> blockchain_account_index_;
    UnallocatedMap<identifier::Account, std::shared_ptr<proto::HDAccount>>
        blockchain_accounts_{};
    Hash issuers_root_;
    mutable std::mutex issuers_lock_;
    mutable std::unique_ptr<tree::Issuers> issuers_;
    Hash workflows_root_;
    mutable std::mutex workflows_lock_;
    mutable std::unique_ptr<tree::PaymentWorkflows> workflows_;
    UnallocatedMap<PurseID, Hash> purse_id_;
    mutable std::mutex ethereum_lock_;
    UnallocatedMap<UnitType, UnallocatedSet<identifier::Account>>
        ethereum_account_types_{};
    UnallocatedMap<identifier::Account, UnitType> ethereum_account_index_;
    UnallocatedMap<
        identifier::Account,
        std::shared_ptr<proto::BlockchainEthereumAccountData>>
        ethereum_accounts_{};

    template <typename T, typename... Args>
    auto construct(
        std::mutex& mutex,
        std::unique_ptr<T>& pointer,
        const Hash& root,
        Args&&... params) const -> T*;

    auto bip47() const -> tree::Bip47Channels*;
    auto dump(const Lock&, const Log&, Vector<Hash>& out) const noexcept
        -> bool final;
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
    auto threads() const -> tree::Threads*;
    auto contexts() const -> tree::Contexts*;
    auto issuers() const -> tree::Issuers*;
    auto workflows() const -> tree::PaymentWorkflows*;

    template <typename T>
    auto editor(Hash& root, std::mutex& mutex, T* (Nym::*get)() const)
        -> Editor<T>;

    auto init(const Hash& hash) noexcept(false) -> void final;
    auto save(const Lock& lock) const -> bool final;
    template <typename O>
    void _save(O* input, const Lock& lock, std::mutex& mutex, Hash& root);
    auto serialize() const -> proto::StorageNym;
    auto upgrade(const Lock& lock) noexcept -> bool final;
};
}  // namespace opentxs::storage::tree
