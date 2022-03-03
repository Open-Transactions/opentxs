// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <tuple>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identity
{
class Nym;
}  // namespace identity

class Identifier;
class Message;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using CredentialIndexModeFlag = bool;
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_ONLY_IDS = true;
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_FULL_CREDS = false;

using CredentialModeFlag = bool;
static const CredentialModeFlag PRIVATE_VERSION = true;
static const CredentialModeFlag PUBLIC_VERSION = false;

using SerializationModeFlag = bool;
static const SerializationModeFlag AS_PRIVATE = true;
static const SerializationModeFlag AS_PUBLIC = false;

using SerializationSignatureFlag = bool;
static const SerializationSignatureFlag WITH_SIGNATURES = true;
static const SerializationSignatureFlag WITHOUT_SIGNATURES = false;

using ProtoValidationVerbosity = bool;
static const ProtoValidationVerbosity SILENT = true;
static const ProtoValidationVerbosity VERBOSE = false;

using BIP44Chain = bool;
static const BIP44Chain INTERNAL_CHAIN = true;
static const BIP44Chain EXTERNAL_CHAIN = false;

using BlockMode = bool;
static const BlockMode BLOCK_MODE = true;
static const BlockMode NOBLOCK_MODE = false;

enum class StringStyle : bool { Hex = true, Raw = false };

using GetPreimage = std::function<UnallocatedCString()>;
using SimpleCallback = std::function<void()>;

using DhtResults = UnallocatedVector<std::shared_ptr<UnallocatedCString>>;

using DhtDoneCallback = std::function<void(bool)>;
using DhtResultsCallback = std::function<bool(const DhtResults&)>;

using PeriodicTask = std::function<void()>;

/** C++11 representation of a claim. This version is more useful than the
 *  protobuf version, since it contains the claim ID.
 */
using Claim = std::tuple<
    UnallocatedCString,              // claim identifier
    std::uint32_t,                   // section
    std::uint32_t,                   // type
    UnallocatedCString,              // value
    std::int64_t,                    // start time
    std::int64_t,                    // end time
    UnallocatedSet<std::uint32_t>>;  // attributes
using ClaimTuple = Claim;

/** C++11 representation of all contact data associated with a nym, aggregating
 *  each the nym's contact credentials in the event it has more than one.
 */
using ClaimSet = UnallocatedSet<Claim>;

/** A list of object IDs and their associated aliases
 *  * string: id of the stored object
 *  * string: alias of the stored object
 */
using ObjectList =
    UnallocatedList<std::pair<UnallocatedCString, UnallocatedCString>>;

using RawData = UnallocatedVector<unsigned char>;

using Nym_p = std::shared_ptr<const identity::Nym>;

// local ID, remote ID
using ContextID = std::pair<UnallocatedCString, UnallocatedCString>;
using ContextLockCallback =
    std::function<std::recursive_mutex&(const ContextID&)>;
using SetID = std::function<void(const Identifier&)>;

using NetworkOperationStatus = std::int32_t;

using Lock = std::unique_lock<std::mutex>;
using rLock = std::unique_lock<std::recursive_mutex>;
using sLock = std::shared_lock<std::shared_mutex>;
using eLock = std::unique_lock<std::shared_mutex>;

enum class ClaimPolarity : std::uint8_t {
    NEUTRAL = 0,
    POSITIVE = 1,
    NEGATIVE = 2
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

enum class EcdsaCurve : std::uint8_t {
    invalid = 0,
    secp256k1 = 1,
    ed25519 = 2,
};

enum class NymCapability : std::uint8_t {
    SIGN_MESSAGE = 0,
    ENCRYPT_MESSAGE = 1,
    AUTHENTICATE_CONNECTION = 2,
    SIGN_CHILDCRED = 3,
};

enum class SendResult : std::int8_t {
    TRANSACTION_NUMBERS = -3,
    INVALID_REPLY = -2,
    TIMEOUT = -1,
    Error = 0,
    UNNECESSARY = 1,
    VALID_REPLY = 2,
    SHUTDOWN = 3,
};

enum class ConnectionState : std::uint8_t {
    NOT_ESTABLISHED = 0,
    ACTIVE = 1,
    STALLED = 2
};

using Endpoint = std::tuple<
    int,                 // address type
    int,                 // protocol version
    UnallocatedCString,  // hostname / address
    std::uint32_t,       // port
    VersionNumber>;
using NetworkReplyRaw =
    std::pair<SendResult, std::shared_ptr<UnallocatedCString>>;
using NetworkReplyString = std::pair<SendResult, std::shared_ptr<String>>;
using NetworkReplyMessage = std::pair<SendResult, std::shared_ptr<Message>>;

using CommandResult =
    std::tuple<RequestNumber, TransactionNumber, NetworkReplyMessage>;

enum class ThreadStatus : std::uint8_t {
    Error = 0,
    RUNNING = 1,
    FINISHED_SUCCESS = 2,
    FINISHED_FAILED = 3,
    SHUTDOWN = 4,
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
}  // namespace opentxs
