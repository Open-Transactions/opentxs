// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

#include "api/session/activity/MailCache.hpp"
#include "internal/api/session/Activity.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Contacts;
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace contact
{
class Contact;
}  // namespace contact

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class StorageThread;
}  // namespace proto

class PeerObject;
}  // namespace opentxs

namespace opentxs::api::session::imp
{
class Activity final : virtual public internal::Activity, Lockable
{
public:
    auto AddBlockchainTransaction(const blockchain::block::bitcoin::Transaction&
                                      transaction) const noexcept -> bool final;
    auto AddPaymentEvent(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const StorageBox type,
        const Identifier& itemID,
        const Identifier& workflowID,
        Time time) const noexcept -> bool final;
    auto Mail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const noexcept -> std::unique_ptr<Message> final
    {
        return mail_.LoadMail(nym, id, box);
    }
    auto Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const StorageBox box,
        const PeerObject& text) const noexcept -> std::string final;
    auto Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const StorageBox box,
        const std::string& text) const noexcept -> std::string final;
    auto Mail(const identifier::Nym& nym, const StorageBox box) const noexcept
        -> ObjectList final;
    auto MailRemove(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const noexcept -> bool final;
    auto MailText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box,
        const PasswordPrompt& reason) const noexcept
        -> std::shared_future<std::string> final
    {
        return mail_.GetText(nym, id, box, reason);
    }
    auto MarkRead(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const noexcept -> bool final;
    auto MarkUnread(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const noexcept -> bool final;
    auto Cheque(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept -> ChequeData final;
    auto Transfer(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept -> TransferData final;
    auto PaymentText(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const noexcept
        -> std::shared_ptr<const std::string> final;
    auto PreloadActivity(
        const identifier::Nym& nymID,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept -> void final;
    auto PreloadThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const std::size_t start,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept -> void final;
    auto Thread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        proto::StorageThread& serialzied) const noexcept -> bool final;
    auto Thread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        AllocateOutput output) const noexcept -> bool final;
    auto Threads(const identifier::Nym& nym, const bool unreadOnly = false)
        const noexcept -> ObjectList final;
    auto UnreadCount(const identifier::Nym& nym) const noexcept
        -> std::size_t final;
    auto ThreadPublisher(const identifier::Nym& nym) const noexcept
        -> std::string final;

    Activity(
        const api::Session& api,
        const session::Contacts& contact) noexcept;

    ~Activity() final;

private:
    const api::Session& api_;
    const session::Contacts& contact_;
    const OTZMQPublishSocket message_loaded_;
    mutable activity::MailCache mail_;
    mutable std::mutex publisher_lock_;
    mutable std::pmr::map<OTIdentifier, OTZMQPublishSocket> thread_publishers_;
    mutable std::pmr::map<OTNymID, OTZMQPublishSocket> blockchain_publishers_;

    auto activity_preload_thread(
        OTPasswordPrompt reason,
        const OTIdentifier nymID,
        const std::size_t count) const noexcept -> void;
    auto thread_preload_thread(
        OTPasswordPrompt reason,
        const std::string nymID,
        const std::string threadID,
        const std::size_t start,
        const std::size_t count) const noexcept -> void;

#if OT_BLOCKCHAIN
    auto add_blockchain_transaction(
        const eLock& lock,
        const identifier::Nym& nym,
        const blockchain::block::bitcoin::Transaction& transaction)
        const noexcept -> bool;
#endif  // OT_BLOCKCHAIN
    auto nym_to_contact(const std::string& nymID) const noexcept
        -> std::shared_ptr<const contact::Contact>;
#if OT_BLOCKCHAIN
    auto get_blockchain(const eLock&, const identifier::Nym& nymID)
        const noexcept -> const opentxs::network::zeromq::socket::Publish&;
#endif  // OT_BLOCKCHAIN
    auto get_publisher(const identifier::Nym& nymID) const noexcept
        -> const opentxs::network::zeromq::socket::Publish&;
    auto get_publisher(const identifier::Nym& nymID, std::string& endpoint)
        const noexcept -> const opentxs::network::zeromq::socket::Publish&;
    auto publish(const identifier::Nym& nymID, const Identifier& threadID)
        const noexcept -> void;
    auto start_publisher(const std::string& endpoint) const noexcept
        -> OTZMQPublishSocket;
    auto verify_thread_exists(const std::string& nym, const std::string& thread)
        const noexcept -> bool;

    Activity() = delete;
    Activity(const Activity&) = delete;
    Activity(Activity&&) = delete;
    auto operator=(const Activity&) -> Activity& = delete;
    auto operator=(Activity&&) -> Activity& = delete;
};
}  // namespace opentxs::api::session::imp
