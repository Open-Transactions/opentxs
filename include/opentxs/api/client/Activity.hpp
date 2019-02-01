// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_ACTIVITY_HPP
#define OPENTXS_API_CLIENT_ACTIVITY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <tuple>

namespace opentxs
{
namespace api
{
namespace client
{
class Activity
{
public:
    using ChequeData = std::pair<
        std::unique_ptr<const class Cheque>,
        std::shared_ptr<const UnitDefinition>>;
    using TransferData = std::pair<
        std::unique_ptr<const opentxs::Item>,
        std::shared_ptr<const UnitDefinition>>;

    EXPORT virtual bool AddBlockchainTransaction(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const StorageBox box,
        const proto::BlockchainTransaction& transaction) const = 0;
    EXPORT virtual bool AddPaymentEvent(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const StorageBox type,
        const Identifier& itemID,
        const Identifier& workflowID,
        std::chrono::time_point<std::chrono::system_clock> time) const = 0;
    EXPORT virtual bool MoveIncomingBlockchainTransaction(
        const identifier::Nym& nymID,
        const Identifier& fromThreadID,
        const Identifier& toThreadID,
        const std::string& txid) const = 0;
    /**   Load a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    EXPORT virtual std::unique_ptr<Message> Mail(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const = 0;
    /**   Store a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    EXPORT virtual std::string Mail(
        const identifier::Nym& nym,
        const Message& mail,
        const StorageBox box) const = 0;
    /**   Obtain a list of mail objects in a specified box
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] box the box to be listed
     */
    EXPORT virtual ObjectList Mail(
        const identifier::Nym& nym,
        const StorageBox box) const = 0;
    /**   Delete a mail object
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] mail the mail object to be stored
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns The id of the stored message. The string will be empty if
     *             the mail object can not be stored.
     */
    EXPORT virtual bool MailRemove(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox box) const = 0;
    /**   Retrieve the text from a message
     *
     *    \param[in] nym the identifier of the nym who owns the mail box
     *    \param[in] id the identifier of the mail object
     *    \param[in] box the box from which to retrieve the mail object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    EXPORT virtual std::shared_ptr<const std::string> MailText(
        const identifier::Nym& nym,
        const Identifier& id,
        const StorageBox& box) const = 0;
    /**   Mark a thread item as read
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked read
     *    \returns False if the nym, thread, or item does not exist
     */
    EXPORT virtual bool MarkRead(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const = 0;
    /**   Mark a thread item as unread
     *
     *    \param[in] nymId the identifier of the nym who owns the thread
     *    \param[in] threadId the thread containing the item to be marked
     *    \param[in] itemId the identifier of the item to be marked unread
     *    \returns False if the nym, thread, or item does not exist
     */
    EXPORT virtual bool MarkUnread(
        const identifier::Nym& nymId,
        const Identifier& threadId,
        const Identifier& itemId) const = 0;

    EXPORT virtual ChequeData Cheque(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const = 0;

    EXPORT virtual TransferData Transfer(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const = 0;

    /**   Summarize a payment workflow event in human-friendly test form
     *
     *    \param[in] nym the identifier of the nym who owns the thread
     *    \param[in] id the identifier of the payment item
     *    \param[in] workflow the identifier of the payment workflow
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    EXPORT virtual std::shared_ptr<const std::string> PaymentText(
        const identifier::Nym& nym,
        const std::string& id,
        const std::string& workflow) const = 0;

    /**   Asynchronously cache the most recent items in each of a nym's threads
     *
     *    \param[in] nymID the identifier of the nym who owns the thread
     *    \param[in] count the number of items to preload in each thread
     */
    EXPORT virtual void PreloadActivity(
        const identifier::Nym& nymID,
        const std::size_t count) const = 0;
    /**   Asynchronously cache the items in an activity thread
     *
     *    \param[in] nymID the identifier of the nym who owns the thread
     *    \param[in] threadID the thread containing the items to be cached
     *    \param[in] start the first item to be cached
     *    \param[in] count the number of items to cache
     */
    EXPORT virtual void PreloadThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const std::size_t start,
        const std::size_t count) const = 0;
    EXPORT virtual std::shared_ptr<proto::StorageThread> Thread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const = 0;
    /**   Obtain a list of thread ids for the specified nym
     *
     *    \param[in] nym the identifier of the nym
     *    \param[in] unreadOnly if true, only return threads with unread items
     */
    EXPORT virtual ObjectList Threads(
        const identifier::Nym& nym,
        const bool unreadOnly = false) const = 0;
    /**   Return the total number of unread thread items for a nym
     *
     *    \param[in] nymId
     */
    EXPORT virtual std::size_t UnreadCount(
        const identifier::Nym& nym) const = 0;

    EXPORT virtual std::string ThreadPublisher(
        const identifier::Nym& nym) const = 0;

    virtual ~Activity() = default;

protected:
    Activity() = default;

private:
    Activity(const Activity&) = delete;
    Activity(Activity&&) = delete;
    Activity& operator=(const Activity&) = delete;
    Activity& operator=(Activity&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
