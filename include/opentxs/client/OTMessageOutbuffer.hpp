// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_OTMESSAGEOUTBUFFER_HPP
#define OPENTXS_CLIENT_OTMESSAGEOUTBUFFER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/String.hpp"

#include <map>

namespace opentxs
{

// OUTOING MESSAGES (from me--client--sent to server.)
//
// The purpose of this class is to cache client requests (being sent to the
// server) so that they can later be queried (using the request number) by the
// developer using the OTAPI, so that if transaction numbers need to be clawed
// back from failed messages, etc, they are available.
//
// The OT client side also can use this as a mechanism to help separate
// old-and-dealt-with messages, by explicitly removing messages from this queue
// once they are dealt with. This way the developer can automatically assume
// that any reply is old if it carries a request number that cannot be found in
// this queue.
class OTMessageOutbuffer : Lockable
{
public:
    EXPORT OTMessageOutbuffer(const api::Core& core);

    EXPORT void Clear(
        const String& notaryID,
        const String& nymId,
        const bool harvestingForRetry,
        ServerContext& context,
        const identifier::Nym& nymID);
    // Allocate theMsg on the heap (takes ownership.) Mapped by request num.
    // Note: AddSentMessage, if it finds a message already on the map with the
    // same request number, deletes the old one before adding the new one. In
    // the future may contemplate using multimap here instead (if completeness
    // becomes desired over uniqueness.)
    EXPORT void AddSentMessage(std::shared_ptr<Message> message);
    // null == not found. caller NOT responsible to delete.
    EXPORT std::shared_ptr<Message> GetSentMessage(
        const std::int64_t& requestNum,
        const String& notaryID,
        const String& nymId);
    // true == it was removed. false == it wasn't found.
    EXPORT bool RemoveSentMessage(
        const std::int64_t& requestNum,
        const String& notaryID,
        const String& nymId);
    // null == not found. caller NOT responsible to delete.
    EXPORT std::shared_ptr<Message> GetSentMessage(
        const OTTransaction& transaction);
    // true == it was removed. false == it wasn't found.
    EXPORT bool RemoveSentMessage(const OTTransaction& transaction);

    EXPORT ~OTMessageOutbuffer();

private:
    typedef std::multimap<std::int64_t, std::shared_ptr<Message>> mapOfMessages;

    const api::Core& api_;
    mapOfMessages messagesMap_{};

    OTMessageOutbuffer() = delete;
    OTMessageOutbuffer(const OTMessageOutbuffer&);
    OTMessageOutbuffer& operator=(const OTMessageOutbuffer&);
};
}  // namespace opentxs
#endif
