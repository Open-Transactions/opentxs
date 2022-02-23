/**
 * @file Types.hpp
 * @brief File containing a useful aliases and enums used in the opentxs namespace.
 * @copyright Copyright (c) 2010-2022 The Open-Transactions developers
 *            This Source Code Form is subject to the terms of the Mozilla Public
 *            License, v. 2.0. If a copy of the MPL was not distributed with this
 *            file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
namespace opentxs
{
namespace identity
{
class Nym;
}  // namespace identity

class Identifier;
class Message;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/**
 * @namespace opentxs
 * @brief Opentxs implementing the OTX protocol as well as a full-strength financial cryptography library.
 */
namespace opentxs
{

/**
 * @brief Used to serialize the Authority class.
 * @see Authority, CREDENTIAL_INDEX_MODE_ONLY_IDS, CREDENTIAL_INDEX_MODE_FULL_CREDS
 */
using CredentialIndexModeFlag = bool;
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_ONLY_IDS = true;     /**< @brief Defined CREDENTIAL_INDEX_MODE_ONLY_IDS as true.
                                                                                            Used to Serialize the Authority class
                                                                                     @see CredentialIndexModeFlag, Authority */
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_FULL_CREDS = false;  /**< @brief Defined CREDENTIAL_INDEX_MODE_FULL_CREDS as false.
                                                                                            Used to Serialize the Authority class
                                                                                     @see CredentialIndexModeFlag, Authority */

/**
 * @brief Used to signature verification in the credential implementation
 * @see PRIVATE_VERSION, PUBLIC_VERSION, Key
 */
using CredentialModeFlag = bool;
static const CredentialModeFlag PRIVATE_VERSION = true; /**< @brief Defined PRIVATE_VERSION as true.
                                                                    Used to signature verification in the credential implementation
                                                             @see CredentialModeFlag, Key */
static const CredentialModeFlag PUBLIC_VERSION = false; /**< @brief Defined PUBLIC_VERSION as false.
                                                                    Used to signature verification in the credential implementation
                                                             @see CredentialModeFlag, Key */

/**
 * @brief SerializationModeFlag
 * @see AS_PRIVATE, AS_PUBLIC
 */
using SerializationModeFlag = bool;
static const SerializationModeFlag AS_PRIVATE = true; /**< @brief Defined AS_PRIVATE as true.
                                                           @see SerializationModeFlag*/
static const SerializationModeFlag AS_PUBLIC = false; /**< @brief Defined AS_PUBLIC as false.
                                                           @see SerializationModeFlag*/

/**
 * @brief SerializationSignatureFlag
 * @see WITH_SIGNATURES, WITHOUT_SIGNATURES
 */
using SerializationSignatureFlag = bool;
static const SerializationSignatureFlag WITH_SIGNATURES = true;     /**< @brief Defined WITH_SIGNATURES as true.
                                                                         @see SerializationSignatureFlag*/
static const SerializationSignatureFlag WITHOUT_SIGNATURES = false; /**< @brief Defined WITHOUT_SIGNATURES as false.
                                                                         @see SerializationSignatureFlag*/

/**
 * @brief ProtoValidationVerbosity
 * @see SILENT, VERBOSE
 */
using ProtoValidationVerbosity = bool;
static const ProtoValidationVerbosity SILENT = true;    /**< @brief Defined SILENT as true.
                                                             @see ProtoValidationVerbosity*/
static const ProtoValidationVerbosity VERBOSE = false;  /**< @brief Defined VERBOSE as false.
                                                             @see ProtoValidationVerbosity*/

/**
 * @brief BIP44Chain
 * @see INTERNAL_CHAIN, EXTERNAL_CHAIN
 */
using BIP44Chain = bool;
static const BIP44Chain INTERNAL_CHAIN = true;  /**< @brief Defined INTERNAL_CHAIN as true.
                                                     @see BIP44Chain*/
static const BIP44Chain EXTERNAL_CHAIN = false; /**< @brief Defined EXTERNAL_CHAIN as false.
                                                     @see BIP44Chain*/

/**
 * @brief BlockMode.
 * @see BLOCK_MODE, NOBLOCK_MODE
 */
using BlockMode = bool;
static const BlockMode BLOCK_MODE = true;       /**< @brief Defined BLOCK_MODE as true.
                                                     @see BlockMode*/
static const BlockMode NOBLOCK_MODE = false;    /**< @brief Defined NOBLOCK_MODE as false.
                                                     @see BlockMode*/

/**
 * @brief The StringStyle is defined as a bool and can take one of two values.
 */
enum class StringStyle : bool {
    Hex = true, /**< @brief Type of StringStyle Hex.*/
    Raw = false /**< @brief Type of StringStyle Raw.*/
};

/**
 * @brief Alias for function pointer.
 * @details Contained function takes no arguments and return UnallocatedCString.
 *          Used in methods to checking credentials.
 * @see UnallocatedCString, crypto, identity
 */
using GetPreimage = std::function<UnallocatedCString()>;

/**
 * @brief Alias for function pointer.
 * @details Contained function takes no arguments and does not return result value.
 * @see
 */
using SimpleCallback = std::function<void()>;

/**
 * @brief Alias to vector of smart pointers to UnallocatedCStrings
 * @see UnallocatedVector, UnallocatedCString
 */
using DhtResults = UnallocatedVector<std::shared_ptr<UnallocatedCString>>;

/**
 * @brief Alias for function pointer.
 * @details Callbac called when dht finish ation.
 * @see network
 */
using DhtDoneCallback = std::function<void(bool)>;

/**
 * @brief Alias for function pointer.
 * @details Callback with result of dht action
 * @see
 */
using DhtResultsCallback = std::function<bool(const DhtResults&)>;

/**
 * @brief Alias for function pointer.
 * @see Periodic
 */
using PeriodicTask = std::function<void()>;

/**
 *  @brief C++11 representation of a claim.
 *  @details This version is more useful than the
 *           protobuf version, since it contains the claim ID.
 *           Contain:
 *           * claim identifier
 *           * section
 *           * type
 *           * value
 *           * start time
 *           * end time
 *           * attributes
 *  @see UnallocatedSet, UnallocatedCString
 */
using Claim = std::tuple<
    UnallocatedCString,              // claim identifier
    std::uint32_t,                   // section
    std::uint32_t,                   // type
    UnallocatedCString,              // value
    std::int64_t,                    // start time
    std::int64_t,                    // end time
    UnallocatedSet<std::uint32_t>>;  // attributes

/**
 * @brief Alias for Claim.
 * @see Claim
 */
using ClaimTuple = Claim;

/**
 *  @brief Alias for container of Claims.
 *  @details C++11 representation of all contact data associated with a nym, aggregating
 *           each the nym's contact credentials in the event it has more than one.
 *  @see Claim, UnallocatedSet
 */
using ClaimSet = UnallocatedSet<Claim>;

/**
 *  @brief Alias for container of pair.
 *  @details A list of object IDs and their associated aliases
 *           * string: id of the stored object
 *           * string: alias of the stored object
 *  @see UnallocatedList, UnallocatedCString
 */
using ObjectList =
    UnallocatedList<std::pair<UnallocatedCString, UnallocatedCString>>;

/**
 * @brief Alias for vector of chars.
 * @see UnallocatedVector
 */
using RawData = UnallocatedVector<unsigned char>;

/**
 * @brief Alias for smart pointer to Nym
 * @see Nym
 */
using Nym_p = std::shared_ptr<const identity::Nym>;

/**
 * @brief Alias for pair of local and remote IDs
 * @see UnallocatedCString
 */
using ContextID = std::pair<UnallocatedCString, UnallocatedCString>;

/**
 * @brief Alias for callback of recursive mutex
 * @see ContextID
 */
using ContextLockCallback =
    std::function<std::recursive_mutex&(const ContextID&)>;

/**
 * @brief Alias for function to SetID
 * @see Identifier
 */
using SetID = std::function<void(const Identifier&)>;

//unused
/**
 * @brief Alias for operation status
 */
using NetworkOperationStatus = std::int32_t;

/**
 * @brief Alias for unique lock
 */
using Lock = std::unique_lock<std::mutex>;

/**
 * @brief Alias for unique lock recursive mutex
 */
using rLock = std::unique_lock<std::recursive_mutex>;

/**
 * @brief Alias for shared mutex
 */
using sLock = std::shared_lock<std::shared_mutex>;

/**
 * @brief Alias for unique lock shared mutex
 */
using eLock = std::unique_lock<std::shared_mutex>;

// not used in the project
/**
 * @brief ClaimPolarity defined as one byte
 */
enum class ClaimPolarity : std::uint8_t {
    NEUTRAL = 0,    /**< @brief Value 0.*/
    POSITIVE = 1,   /**< @brief Value 1.*/
    NEGATIVE = 2    /**< @brief Value 2.*/
};

/**
 * @brief StorageBox defined as one byte.
 * @details StorageBox describes type of peer.
 */
enum class StorageBox : std::uint8_t {
    SENTPEERREQUEST = 0,        /**< @brief Value 0.*/
    INCOMINGPEERREQUEST = 1,    /**< @brief Value 1.*/
    SENTPEERREPLY = 2,          /**< @brief Value 2.*/
    INCOMINGPEERREPLY = 3,      /**< @brief Value 3.*/
    FINISHEDPEERREQUEST = 4,    /**< @brief Value 4.*/
    FINISHEDPEERREPLY = 5,      /**< @brief Value 5.*/
    PROCESSEDPEERREQUEST = 6,   /**< @brief Value 6.*/
    PROCESSEDPEERREPLY = 7,     /**< @brief Value 7.*/
    MAILINBOX = 8,              /**< @brief Value 8.*/
    MAILOUTBOX = 9,             /**< @brief Value 9.*/
    BLOCKCHAIN = 10,            /**< @brief Value 10.*/
    RESERVED_1 = 11,            /**< @brief Value 11.*/
    INCOMINGCHEQUE = 12,        /**< @brief Value 12.*/
    OUTGOINGCHEQUE = 13,        /**< @brief Value 13.*/
    OUTGOINGTRANSFER = 14,      /**< @brief Value 14.*/
    INCOMINGTRANSFER = 15,      /**< @brief Value 15.*/
    INTERNALTRANSFER = 16,      /**< @brief Value 16.*/
    PENDING_SEND = 253,         /**< @brief Value 253.*/
    DRAFT = 254,                /**< @brief Value 254.*/
    UNKNOWN = 255,              /**< @brief Value 255.*/
};

/**
 * @brief EcdsaCurve defined as one byte
 * @details Enum used in cryptography parts
 * @see crypto, session
 */
enum class EcdsaCurve : std::uint8_t {
    invalid = 0,    /**< @brief Value 0.*/
    secp256k1 = 1,  /**< @brief Value 1.*/
    ed25519 = 2,    /**< @brief Value 2.*/
};

/**
 * @brief NymCapability defined as one byte
 */
enum class NymCapability : std::uint8_t {
    SIGN_MESSAGE = 0,               /**< @brief Value 0.*/
    ENCRYPT_MESSAGE = 1,            /**< @brief Value 1.*/
    AUTHENTICATE_CONNECTION = 2,    /**< @brief Value 2.*/
    SIGN_CHILDCRED = 3,             /**< @brief Value 3.*/
};

/**
 * @brief SendResult defined as one byte
 */
enum class SendResult : std::int8_t {
    TRANSACTION_NUMBERS = -3,   /**< @brief Value -3.*/
    INVALID_REPLY = -2,         /**< @brief Value -2.*/
    TIMEOUT = -1,               /**< @brief Value -1.*/
    Error = 0,                  /**< @brief Value 0.*/
    UNNECESSARY = 1,            /**< @brief Value 1.*/
    VALID_REPLY = 2,            /**< @brief Value 2.*/
    SHUTDOWN = 3,               /**< @brief Value 3.*/
};

/**
 * @brief ConnectionState defined as one byte
 */
enum class ConnectionState : std::uint8_t {
    NOT_ESTABLISHED = 0,    /**< @brief Value 0.*/
    ACTIVE = 1,             /**< @brief Value 1.*/
    STALLED = 2             /**< @brief Value 2.*/
};

/**
 * @brief Alias for tuple with protocol details.
 */
using Endpoint = std::tuple<
    int,                 // address type
    int,                 // protocol version
    UnallocatedCString,  // hostname / address
    std::uint32_t,       // port
    VersionNumber>;

//unused
/**
 * @brief Alias for pair for network reply
 */
using NetworkReplyRaw =
    std::pair<SendResult, std::shared_ptr<UnallocatedCString>>;

//unused
/**
 * @brief Alias for pair for network reply
 */
using NetworkReplyString = std::pair<SendResult, std::shared_ptr<String>>;

/**
 * @brief Alias for pair message and result used to network communication
 * @see SendResult, Message, network
 */
using NetworkReplyMessage = std::pair<SendResult, std::shared_ptr<Message>>;

/**
 * @brief Alias for tulpe used in the network communication
 * @see OT_API, RequestNumber, TransactionNumber, NetworkReplyMessage
 */
using CommandResult =
    std::tuple<RequestNumber, TransactionNumber, NetworkReplyMessage>;

/**
 * @brief ThreadStatus defined as one byte
 */
enum class ThreadStatus : std::uint8_t {
    Error = 0,              /**< @brief Value 0.*/
    RUNNING = 1,            /**< @brief Value 1.*/
    FINISHED_SUCCESS = 2,   /**< @brief Value 2.*/
    FINISHED_FAILED = 3,    /**< @brief Value 3.*/
    SHUTDOWN = 4            /**< @brief Value 4.*/
};

/**
 * @brief Messagability defined as one byte
 */
enum class Messagability : std::int8_t {
    MISSING_CONTACT = -5,   /**< @brief Value -5.*/
    CONTACT_LACKS_NYM = -4, /**< @brief Value -4.*/
    NO_SERVER_CLAIM = -3,   /**< @brief Value -3.*/
    INVALID_SENDER = -2,    /**< @brief Value -2.*/
    MISSING_SENDER = -1,    /**< @brief Value -1.*/
    READY = 0,              /**< @brief Value 0.*/
    MISSING_RECIPIENT = 1,  /**< @brief Value 1.*/
    UNREGISTERED = 2        /**< @brief Value 2.*/
};

/**
 * @brief Depositability defined as one byte
 */
enum class Depositability : std::int8_t {
    ACCOUNT_NOT_SPECIFIED = -4, /**< @brief Value -4.*/
    WRONG_ACCOUNT = -3,         /**< @brief Value -3.*/
    WRONG_RECIPIENT = -2,       /**< @brief Value -2.*/
    INVALID_INSTRUMENT = -1,    /**< @brief Value -1.*/
    READY = 0,                  /**< @brief Value 0.*/
    NOT_REGISTERED = 1,         /**< @brief Value 1.*/
    NO_ACCOUNT = 2,             /**< @brief Value 2.*/
    UNKNOWN = 127               /**< @brief Value 127.*/
};

/**
 * @brief RemoteBoxType defined as one byte
 */
enum class RemoteBoxType : std::int8_t {
    Error = -1,     /**< @brief Value -1.*/
    Nymbox = 0,     /**< @brief Value 0.*/
    Inbox = 1,      /**< @brief Value 1.*/
    Outbox = 2      /**< @brief Value 2.*/
};

/**
 * @brief PaymentType defined as int
 */
enum class PaymentType : int {
    Error = 0,      /**< @brief Value 0.*/
    Cheque = 1,     /**< @brief Value 1.*/
    Voucher = 2,    /**< @brief Value 2.*/
    Transfer = 3,   /**< @brief Value 3.*/
    Blinded = 4     /**< @brief Value 4.*/
};
}  // namespace opentxs
