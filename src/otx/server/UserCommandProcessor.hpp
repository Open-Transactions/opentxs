// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "internal/otx/common/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Notary;
class Wallet;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Account;
class Generic;
class Notary;
class Nym;
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
class ReplyMessage;
class Server;
}  // namespace server

class Ledger;
class NumList;
class OTTransaction;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::server
{
class UserCommandProcessor
{
public:
    Server& server_;

    static auto check_client_isnt_server(
        const identifier::Nym& nymID,
        const identity::Nym& serverNym) -> bool;
    static auto check_message_notary(
        const identifier::Notary& notaryID,
        const identifier::Generic& realNotaryID) -> bool;
    static auto check_server_lock(const identifier::Nym& nymID) -> bool;
    static auto isAdmin(const identifier::Nym& nymID) -> bool;

    void drop_reply_notice_to_nymbox(
        const api::session::Wallet& wallet,
        const Message& message,
        const std::int64_t& requestNum,
        const bool replyTransSuccess,
        otx::context::Client& context,
        Server& server) const;

    auto ProcessUserCommand(const Message& msgIn, Message& msgOut) -> bool;

    UserCommandProcessor() = delete;
    UserCommandProcessor(const UserCommandProcessor&) = delete;
    UserCommandProcessor(UserCommandProcessor&&) = delete;
    auto operator=(const UserCommandProcessor&)
        -> UserCommandProcessor& = delete;
    auto operator=(UserCommandProcessor&&) -> UserCommandProcessor& = delete;

private:
    friend Server;

    class FinalizeResponse
    {
    public:
        FinalizeResponse(
            const api::Session& api,
            const identity::Nym& nym,
            ReplyMessage& reply,
            Ledger& ledger);
        FinalizeResponse() = delete;
        FinalizeResponse(const FinalizeResponse&) = delete;
        FinalizeResponse(FinalizeResponse&&) = delete;
        auto operator=(const FinalizeResponse&) -> FinalizeResponse& = delete;
        auto operator=(FinalizeResponse&&) -> FinalizeResponse& = delete;

        auto AddResponse(std::shared_ptr<OTTransaction> response)
            -> std::shared_ptr<OTTransaction>&;

        ~FinalizeResponse();

    private:
        const api::Session& api_;
        const identity::Nym& nym_;
        ReplyMessage& reply_;
        Ledger& ledger_;
        UnallocatedVector<std::shared_ptr<OTTransaction>> response_;
    };

    const PasswordPrompt& reason_;
    const api::session::Notary& manager_;

    auto add_numbers_to_nymbox(
        const TransactionNumber transactionNumber,
        const NumList& newNumbers,
        bool& savedNymbox,
        Ledger& nymbox,
        identifier::Generic& nymboxHash) const -> bool;
    void check_acknowledgements(ReplyMessage& reply) const;
    auto check_client_nym(ReplyMessage& reply) const -> bool;
    auto check_ping_notary(const Message& msgIn) const -> bool;
    auto check_request_number(
        const Message& msgIn,
        const RequestNumber& correctNumber) const -> bool;
    auto check_usage_credits(ReplyMessage& reply) const -> bool;
    auto cmd_add_claim(ReplyMessage& reply) const -> bool;
    auto cmd_check_nym(ReplyMessage& reply) const -> bool;
    auto cmd_delete_asset_account(ReplyMessage& reply) const -> bool;
    auto cmd_delete_user(ReplyMessage& reply) const -> bool;
    auto cmd_get_account_data(ReplyMessage& reply) const -> bool;
    auto cmd_get_box_receipt(ReplyMessage& reply) const -> bool;
    // Get the publicly-available list of offers on a specific market.
    auto cmd_get_instrument_definition(ReplyMessage& reply) const -> bool;
    // Get the list of markets on this server.
    auto cmd_get_market_list(ReplyMessage& reply) const -> bool;
    auto cmd_get_market_offers(ReplyMessage& reply) const -> bool;
    // Get a report of recent trades that have occurred on a specific market.
    auto cmd_get_market_recent_trades(ReplyMessage& reply) const -> bool;
    auto cmd_get_mint(ReplyMessage& reply) const -> bool;
    auto cmd_get_nym_market_offers(ReplyMessage& reply) const -> bool;
    // Get the offers that a specific Nym has placed on a specific market.
    auto cmd_get_nymbox(ReplyMessage& reply) const -> bool;
    auto cmd_get_request_number(ReplyMessage& reply) const -> bool;
    auto cmd_get_transaction_numbers(ReplyMessage& reply) const -> bool;
    auto cmd_issue_basket(ReplyMessage& reply) const -> bool;
    auto cmd_notarize_transaction(ReplyMessage& reply) const -> bool;
    auto cmd_ping_notary(ReplyMessage& reply) const -> bool;
    auto cmd_process_inbox(ReplyMessage& reply) const -> bool;
    auto cmd_process_nymbox(ReplyMessage& reply) const -> bool;
    auto cmd_query_instrument_definitions(ReplyMessage& reply) const -> bool;
    auto cmd_register_account(ReplyMessage& reply) const -> bool;
    auto cmd_register_contract(ReplyMessage& reply) const -> bool;
    auto cmd_register_instrument_definition(ReplyMessage& reply) const -> bool;
    auto cmd_register_nym(ReplyMessage& reply) const -> bool;
    auto cmd_request_admin(ReplyMessage& reply) const -> bool;
    auto cmd_send_nym_message(ReplyMessage& reply) const -> bool;
    auto cmd_trigger_clause(ReplyMessage& reply) const -> bool;
    auto cmd_usage_credits(ReplyMessage& reply) const -> bool;
    auto create_nymbox(
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const identity::Nym& serverNym) const -> std::unique_ptr<Ledger>;
    auto hash_check(
        const otx::context::Client& context,
        identifier::Generic& nymboxHash) const -> bool;
    auto initialize_request_number(otx::context::Client& context) const
        -> RequestNumber;
    auto load_inbox(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const identifier::Notary& serverID,
        const identity::Nym& serverNym,
        const bool verifyAccount) const -> std::unique_ptr<Ledger>;
    auto load_nymbox(
        const identifier::Nym& nymID,
        const identifier::Notary& serverID,
        const identity::Nym& serverNym,
        const bool verifyAccount) const -> std::unique_ptr<Ledger>;
    auto load_outbox(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const identifier::Notary& serverID,
        const identity::Nym& serverNym,
        const bool verifyAccount) const -> std::unique_ptr<Ledger>;
    auto reregister_nym(ReplyMessage& reply) const -> bool;
    auto save_box(const identity::Nym& nym, Ledger& box) const -> bool;
    auto save_inbox(
        const identity::Nym& nym,
        identifier::Generic& hash,
        Ledger& inbox) const -> bool;
    auto save_nymbox(
        const identity::Nym& nym,
        identifier::Generic& hash,
        Ledger& nymbox) const -> bool;
    auto save_outbox(
        const identity::Nym& nym,
        identifier::Generic& hash,
        Ledger& outbox) const -> bool;
    auto send_message_to_nym(
        const identifier::Notary& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const Message& msg) const -> bool;
    auto verify_box(
        const identifier::Generic& ownerID,
        Ledger& box,
        const identity::Nym& nym,
        const bool full) const -> bool;
    auto verify_transaction(
        const OTTransaction* transaction,
        const identity::Nym& signer) const -> bool;

    UserCommandProcessor(
        Server& server,
        const PasswordPrompt& reason,
        const opentxs::api::session::Notary& manager);
};
}  // namespace opentxs::server
