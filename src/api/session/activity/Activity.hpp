// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <future>
#include <memory>
#include <mutex>

#include "api/session/activity/MailCache.hpp"
#include "internal/api/session/Activity.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Client;
class Contacts;
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace blockchain

namespace protobuf
{
class StorageThread;
}  // namespace protobuf

class Contact;
class PasswordPrompt;
class PeerObject;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class Activity final : virtual public internal::Activity, Lockable
{
public:
    auto AddBlockchainTransaction(
        const api::crypto::Blockchain& crypto,
        const blockchain::block::Transaction& transaction) const noexcept
        -> bool final;
    auto AddPaymentEvent(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const otx::client::StorageBox type,
        const identifier::Generic& itemID,
        const identifier::Generic& workflowID,
        Time time) const noexcept -> bool final;
    auto Cheque(const identifier::Nym& nym, const identifier::Generic& workflow)
        const noexcept -> ChequeData final;
    auto Mail(
        const identifier::Nym& nym,
        const identifier::Generic& id,
        const otx::client::StorageBox& box) const noexcept
        -> std::unique_ptr<Message> final
    {
        return mail_.LoadMail(nym, id, box);
    }
    auto Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const otx::client::StorageBox box,
        const PeerObject& text) const noexcept -> identifier::Generic final;
    auto Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const otx::client::StorageBox box,
        const UnallocatedCString& text) const noexcept
        -> identifier::Generic final;
    auto Mail(const identifier::Nym& nym, const otx::client::StorageBox box)
        const noexcept -> ObjectList final;
    auto MailRemove(
        const identifier::Nym& nym,
        const identifier::Generic& id,
        const otx::client::StorageBox box) const noexcept -> bool final;
    auto MailText(
        const identifier::Nym& nym,
        const identifier::Generic& id,
        const otx::client::StorageBox& box,
        const PasswordPrompt& reason) const noexcept
        -> std::shared_future<UnallocatedCString> final
    {
        return mail_.GetText(nym, id, box, reason);
    }
    auto MarkRead(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        const identifier::Generic& itemId) const noexcept -> bool final;
    auto MarkUnread(
        const identifier::Nym& nymId,
        const identifier::Generic& threadId,
        const identifier::Generic& itemId) const noexcept -> bool final;
    auto Transfer(
        const identifier::Nym& nym,
        const UnallocatedCString& id,
        const identifier::Generic& workflow) const noexcept
        -> TransferData final;
    auto PaymentText(
        const identifier::Nym& nym,
        const UnallocatedCString& id,
        const identifier::Generic& workflow) const noexcept
        -> std::shared_ptr<const UnallocatedCString> final;
    auto PreloadActivity(
        const identifier::Nym& nymID,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept -> void final;
    auto PreloadThread(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const std::size_t start,
        const std::size_t count,
        const PasswordPrompt& reason) const noexcept -> void final;
    auto Thread(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        protobuf::StorageThread& serialzied) const noexcept -> bool final;
    auto Thread(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        Writer&& output) const noexcept -> bool final;
    auto Threads(const identifier::Nym& nym, const bool unreadOnly = false)
        const noexcept -> ObjectList final;
    auto UnreadCount(const identifier::Nym& nym) const noexcept
        -> std::size_t final;
    auto ThreadPublisher(const identifier::Nym& nym) const noexcept
        -> UnallocatedCString final;

    Activity(
        const api::session::Client& api,
        const session::Contacts& contact) noexcept;
    Activity() = delete;
    Activity(const Activity&) = delete;
    Activity(Activity&&) = delete;
    auto operator=(const Activity&) -> Activity& = delete;
    auto operator=(Activity&&) -> Activity& = delete;

    ~Activity() final;

private:
    const api::session::Client& api_;
    const session::Contacts& contact_;
    const OTZMQPublishSocket message_loaded_;
    mutable activity::MailCache mail_;
    mutable std::mutex publisher_lock_;
    mutable UnallocatedMap<identifier::Generic, OTZMQPublishSocket>
        thread_publishers_;
    mutable UnallocatedMap<identifier::Nym, OTZMQPublishSocket>
        blockchain_publishers_;

    auto activity_preload_thread(
        PasswordPrompt reason,
        const identifier::Nym nymID,
        const std::size_t count) const noexcept -> void;
    auto thread_preload_thread(
        PasswordPrompt reason,
        const identifier::Nym nymID,
        const identifier::Generic threadID,
        const std::size_t start,
        const std::size_t count) const noexcept -> void;

    auto add_blockchain_transaction(
        const eLock& lock,
        const identifier::Nym& nym,
        const blockchain::block::Transaction& transaction) const noexcept
        -> bool;
    auto nym_to_contact(const identifier::Nym& nymID) const noexcept
        -> std::shared_ptr<const Contact>;
    auto get_blockchain(const eLock&, const identifier::Nym& nymID)
        const noexcept -> const opentxs::network::zeromq::socket::Publish&;
    auto get_publisher(const identifier::Nym& nymID) const noexcept
        -> const opentxs::network::zeromq::socket::Publish&;
    auto get_publisher(
        const identifier::Nym& nymID,
        UnallocatedCString& endpoint) const noexcept
        -> const opentxs::network::zeromq::socket::Publish&;
    auto publish(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID) const noexcept -> void;
    auto start_publisher(const UnallocatedCString& endpoint) const noexcept
        -> OTZMQPublishSocket;
    auto verify_thread_exists(
        const identifier::Nym& nym,
        const identifier::Generic& thread) const noexcept -> bool;
};
}  // namespace opentxs::api::session::imp
