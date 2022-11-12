// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::MessageType

#pragma once

#include <cstdint>
#include <memory>

#include "internal/otx/Types.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Wallet;
}  // namespace session
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Client;
}  // namespace context
}  // namespace otx

namespace server
{
class Server;
class UserCommandProcessor;
}  // namespace server

class Armored;
class Data;
class PasswordPrompt;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::server
{
class ReplyMessage
{
public:
    auto Acknowledged() const -> UnallocatedSet<RequestNumber>;
    auto HaveContext() const -> bool;
    auto Init() const -> const bool&;
    auto Original() const -> const Message&;
    auto Success() const -> const bool&;

    auto Context() -> otx::context::Client&;
    void ClearRequest();
    void DropToNymbox(const bool success);
    auto LoadContext(const PasswordPrompt& reason) -> bool;
    void OverrideType(const String& accountID);
    void SetAccount(const String& accountID);
    void SetAcknowledgments(const otx::context::Client& context);
    void SetBool(const bool value);
    void SetDepth(const std::int64_t depth);
    void SetEnum(const std::uint8_t value);
    void SetInboxHash(const identifier::Generic& hash);
    void SetInstrumentDefinitionID(const String& id);
    void SetNymboxHash(const identifier::Generic& hash);
    void SetOutboxHash(const identifier::Generic& hash);
    auto SetPayload(const String& payload) -> bool;
    auto SetPayload(const Data& payload) -> bool;
    void SetPayload(const Armored& payload);
    auto SetPayload2(const String& payload) -> bool;
    auto SetPayload3(const String& payload) -> bool;
    void SetRequestNumber(const RequestNumber number);
    void SetSuccess(const bool success);
    void SetTargetNym(const String& nymID);
    void SetTransactionNumber(const TransactionNumber& number);

    ReplyMessage(
        const UserCommandProcessor& parent,
        const opentxs::api::session::Wallet& wallet,
        const identifier::Notary& notaryID,
        const identity::Nym& signer,
        const Message& input,
        Server& server,
        const MessageType& type,
        Message& output,
        const PasswordPrompt& reason);
    ReplyMessage() = delete;
    ReplyMessage(const ReplyMessage&) = delete;
    ReplyMessage(ReplyMessage&&) = delete;
    auto operator=(const ReplyMessage&) -> ReplyMessage& = delete;
    auto operator=(ReplyMessage&&) -> ReplyMessage& = delete;

    ~ReplyMessage();

private:
    const UserCommandProcessor& parent_;
    const opentxs::api::session::Wallet& wallet_;
    const identity::Nym& signer_;
    const Message& original_;
    const PasswordPrompt& reason_;
    const identifier::Notary notary_id_;
    Message& message_;
    Server& server_;
    bool init_{false};
    bool drop_{false};
    bool drop_status_{false};
    Nym_p sender_nym_{nullptr};
    std::unique_ptr<Editor<otx::context::Client>> context_{nullptr};

    void attach_request();
    void clear_request();
    auto init() -> bool;
    auto init_nym() -> bool;
};
}  // namespace opentxs::server
