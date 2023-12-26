// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>
#include <utility>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/NumList.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class FactoryPrivate;
}  // namespace session

class Session;
}  // namespace api

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Base;
class Server;
}  // namespace context
}  // namespace otx

class Message;
class PasswordPrompt;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OTMessageStrategy
{
public:
    virtual auto processXml(Message& message, irr::io::IrrXMLReader*& xml)
        -> std::int32_t = 0;
    virtual void writeXml(Message& message, Tag& parent) = 0;
    virtual ~OTMessageStrategy();

    void processXmlSuccess(Message& m, irr::io::IrrXMLReader*& xml);
};

class OTMessageStrategyManager
{
public:
    auto findStrategy(UnallocatedCString name) -> OTMessageStrategy*
    {
        auto strategy = mapping_.find(name);
        if (strategy == mapping_.end()) { return nullptr; }
        return strategy->second.get();
    }
    void registerStrategy(UnallocatedCString name, OTMessageStrategy* strategy)
    {
        mapping_[name] = std::unique_ptr<OTMessageStrategy>(strategy);
    }

    OTMessageStrategyManager()
        : mapping_()
    {
    }

private:
    UnallocatedUnorderedMap<
        UnallocatedCString,
        std::unique_ptr<OTMessageStrategy>>
        mapping_;
};

class Message final : public Contract
{
protected:
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t final;

    void UpdateContents(const PasswordPrompt& reason) final;

    bool is_signed_{false};

private:
    friend api::session::FactoryPrivate;

    using TypeMap = UnallocatedMap<otx::MessageType, UnallocatedCString>;
    using ReverseTypeMap = UnallocatedMap<UnallocatedCString, otx::MessageType>;

    static const TypeMap message_names_;
    static const ReverseTypeMap message_types_;
    static const UnallocatedMap<otx::MessageType, otx::MessageType>
        reply_message_;

    static auto make_reverse_map() -> ReverseTypeMap;
    static auto reply_command(const otx::MessageType& type) -> otx::MessageType;

    Message(const api::Session& api);

    auto updateContentsByType(Tag& parent) -> bool;

    auto processXmlNodeAckReplies(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t;
    auto processXmlNodeAcknowledgedReplies(
        Message& m,
        irr::io::IrrXMLReader*& xml) -> std::int32_t;
    auto processXmlNodeNotaryMessage(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t;

public:
    static auto Command(const otx::MessageType type) -> UnallocatedCString;
    static auto Type(const UnallocatedCString& type) -> otx::MessageType;
    static auto ReplyCommand(const otx::MessageType type) -> UnallocatedCString;

    ~Message() final;

    auto VerifyContractID() const -> bool final;

    auto SignContract(const identity::Nym& theNym, const PasswordPrompt& reason)
        -> bool final;
    auto VerifySignature(const identity::Nym& theNym) const -> bool final;

    auto HarvestTransactionNumbers(
        otx::context::Server& context,
        bool bHarvestingForRetry,     // false until positively asserted.
        bool bReplyWasSuccess,        // false until positively asserted.
        bool bReplyWasFailure,        // false until positively asserted.
        bool bTransactionWasSuccess,  // false until positively asserted.
        bool bTransactionWasFailure) const -> bool;  // false until positively
                                                     // asserted.

    // So the message can get the list of numbers from the Nym, before sending,
    // that should be listed as acknowledged that the server reply has already
    // been seen for those request numbers.
    void SetAcknowledgments(const otx::context::Base& context);
    void SetAcknowledgments(const UnallocatedSet<RequestNumber>& numbers);

    static void registerStrategy(
        UnallocatedCString name,
        OTMessageStrategy* strategy);

    OTString command_;    // perhaps @register is the string for "reply to
                          // register" a-ha
    OTString notary_id_;  // This is sent with every message for security
                          // reasons.
    OTString nym_id_;     // The hash of the user's public key... or x509 cert.
    OTString nymbox_hash_;  // Sometimes in a server reply as FYI, sometimes
                            // in user message for validation purposes.
    OTString inbox_hash_;   // Sometimes in a server reply as FYI, sometimes
                            // in user message for validation purposes.
    OTString outbox_hash_;  // Sometimes in a server reply as FYI, sometimes
                            // in user message for validation purposes.
    OTString nym_id2_;      // If the user requests public key of another user.
                            // ALSO used for MARKET ID sometimes.
    OTString nym_public_key_;  // The user's public key... or x509 cert.
    OTString instrument_definition_id_;  // The hash of the contract for
                                         // whatever
                                         // digital
                                         // asset is referenced.
    OTString acct_id_;                   // The unique ID of an asset account.
    OTString type_;                      // .
    OTString request_num_;  // Every user has a request number. This prevents
                            // messages from
                            // being intercepted and repeated by attackers.

    OTArmored in_reference_to_;  // If the server responds to a user
                                 // command, he sends
    // it back to the user here in ascii armored format.
    OTArmored payload_;  // If the reply needs to include a payload (such
                         // as a new account
    // or a message envelope or request from another user etc) then
    // it can be put here in ascii-armored format.
    OTArmored payload2_;  // Sometimes one payload just isn't enough.
    OTArmored payload3_;  // Sometimes two payload just isn't enough.

    // This list of request numbers is stored for optimization, so client/server
    // can communicate about
    // which messages have been received, and can avoid certain downloads, such
    // as replyNotice Box Receipts.
    //
    NumList acknowledged_replies_;  // Client request: list of server replies
                                    // client has already seen.
    // Server reply:   list of client-acknowledged replies (so client knows that
    // server knows.)

    std::int64_t new_request_num_{0};  // If you are SENDING a message, you set
                                       // request_num_. (For all msgs.)
    // Server Reply for all messages copies that same number into
    // request_num_;
    // But if this is a SERVER REPLY to the "getRequestNumber" MESSAGE, the
    // "request number" expected in that reply is stored HERE in
    // new_request_num_;
    std::int64_t depth_{0};  // For Market-related messages... (Plus for usage
                             // credits.) Also used by getBoxReceipt
    std::int64_t transaction_num_{0};  // For Market-related messages... Also
                                       // used by getBoxReceipt

    std::uint8_t enum_{0};
    std::uint32_t enum2_{0};

    bool success_{false};  // When the server replies to the client, this may
                           // be true or false
    bool bool_{false};  // Some commands need to send a bool. This variable is
                        // for those.
    std::int64_t time_{0};  // Timestamp when the message was signed.

    static OTMessageStrategyManager messageStrategyManager;
};

class RegisterStrategy
{
public:
    RegisterStrategy(UnallocatedCString name, OTMessageStrategy* strategy)
    {
        Message::registerStrategy(name, strategy);
    }
};
}  // namespace opentxs
