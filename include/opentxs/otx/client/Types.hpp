// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <tuple>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

class Message;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client
{
enum class PaymentWorkflowType : std::uint8_t;
enum class PaymentWorkflowState : std::uint8_t;
enum class SendResult : std::int8_t {
    TRANSACTION_NUMBERS = -3,
    INVALID_REPLY = -2,
    TIMEOUT = -1,
    Error = 0,
    UNNECESSARY = 1,
    VALID_REPLY = 2,
    SHUTDOWN = 3,
};
enum class Messagability : std::int8_t {
    MISSING_CONTACT = -5,
    CONTACT_LACKS_NYM = -4,
    NO_SERVER_CLAIM = -3,
    INVALID_SENDER = -2,
    MISSING_SENDER = -1,
    READY = 0,
    MISSING_RECIPIENT = 1,
    UNREGISTERED = 2,
};
enum class Depositability : std::int8_t {
    ACCOUNT_NOT_SPECIFIED = -4,
    WRONG_ACCOUNT = -3,
    WRONG_RECIPIENT = -2,
    INVALID_INSTRUMENT = -1,
    READY = 0,
    NOT_REGISTERED = 1,
    NO_ACCOUNT = 2,
    UNKNOWN = 127,
};
enum class RemoteBoxType : std::int8_t {
    Error = -1,
    Nymbox = 0,
    Inbox = 1,
    Outbox = 2,
};
enum class PaymentType : int {
    Error = 0,
    Cheque = 1,
    Voucher = 2,
    Transfer = 3,
    Blinded = 4,
};
enum class StorageBox : std::uint8_t {
    SENTPEERREQUEST = 0,
    INCOMINGPEERREQUEST = 1,
    SENTPEERREPLY = 2,
    INCOMINGPEERREPLY = 3,
    FINISHEDPEERREQUEST = 4,
    FINISHEDPEERREPLY = 5,
    PROCESSEDPEERREQUEST = 6,
    PROCESSEDPEERREPLY = 7,
    MAILINBOX = 8,
    MAILOUTBOX = 9,
    BLOCKCHAIN = 10,
    RESERVED_1 = 11,
    INCOMINGCHEQUE = 12,
    OUTGOINGCHEQUE = 13,
    OUTGOINGTRANSFER = 14,
    INCOMINGTRANSFER = 15,
    INTERNALTRANSFER = 16,
    PENDING_SEND = 253,
    DRAFT = 254,
    UNKNOWN = 255,
};
enum class ThreadStatus : std::uint8_t {
    Error = 0,
    RUNNING = 1,
    FINISHED_SUCCESS = 2,
    FINISHED_FAILED = 3,
    SHUTDOWN = 4,
};

using NetworkOperationStatus = std::int32_t;
using NetworkReplyMessage = std::pair<SendResult, std::shared_ptr<Message>>;
using SetID = std::function<void(const identifier::Generic&)>;
}  // namespace opentxs::otx::client
