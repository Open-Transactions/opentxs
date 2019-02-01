// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{
namespace server
{
class UserCommandProcessor
{
public:
    static bool check_client_isnt_server(
        const identifier::Nym& nymID,
        const Nym& serverNym);
    static bool check_message_notary(
        const Identifier& notaryID,
        const Identifier& realNotaryID);
    static bool check_server_lock(const identifier::Nym& nymID);
    static bool isAdmin(const identifier::Nym& nymID);

    void drop_reply_notice_to_nymbox(
        const api::Wallet& wallet,
        const Message& message,
        const std::int64_t& requestNum,
        const bool replyTransSuccess,
        ClientContext& context,
        Server& server) const;

    bool ProcessUserCommand(const Message& msgIn, Message& msgOut);

private:
    friend class Server;

    class FinalizeResponse
    {
    public:
        FinalizeResponse(
            const api::Core& core,
            const Nym& nym,
            ReplyMessage& reply,
            Ledger& ledger);
        FinalizeResponse() = delete;
        FinalizeResponse(const FinalizeResponse&) = delete;
        FinalizeResponse(FinalizeResponse&&) = delete;
        FinalizeResponse& operator=(const FinalizeResponse&) = delete;
        FinalizeResponse& operator=(FinalizeResponse&&) = delete;

        std::shared_ptr<OTTransaction>& AddResponse(
            std::shared_ptr<OTTransaction> response);

        ~FinalizeResponse();

    private:
        const api::Core& api_;
        const Nym& nym_;
        ReplyMessage& reply_;
        Ledger& ledger_;
        std::vector<std::shared_ptr<OTTransaction>> response_;
    };

    Server& server_;
    const opentxs::api::server::Manager& manager_;

    bool add_numbers_to_nymbox(
        const TransactionNumber transactionNumber,
        const NumList& newNumbers,
        bool& savedNymbox,
        Ledger& nymbox,
        Identifier& nymboxHash) const;
    void check_acknowledgements(ReplyMessage& reply) const;
    bool check_client_nym(ReplyMessage& reply) const;
    bool check_ping_notary(const Message& msgIn) const;
    bool check_request_number(
        const Message& msgIn,
        const RequestNumber& correctNumber) const;
    bool check_usage_credits(ReplyMessage& reply) const;
    bool cmd_add_claim(ReplyMessage& reply) const;
    bool cmd_check_nym(ReplyMessage& reply) const;
    bool cmd_delete_asset_account(ReplyMessage& reply) const;
    bool cmd_delete_user(ReplyMessage& reply) const;
    bool cmd_get_account_data(ReplyMessage& reply) const;
    bool cmd_get_box_receipt(ReplyMessage& reply) const;
    // Get the publicly-available list of offers on a specific market.
    bool cmd_get_instrument_definition(ReplyMessage& reply) const;
    // Get the list of markets on this server.
    bool cmd_get_market_list(ReplyMessage& reply) const;
    bool cmd_get_market_offers(ReplyMessage& reply) const;
    // Get a report of recent trades that have occurred on a specific market.
    bool cmd_get_market_recent_trades(ReplyMessage& reply) const;
#if OT_CASH
    bool cmd_get_mint(ReplyMessage& reply) const;
#endif  // OT_CASH
    bool cmd_get_nym_market_offers(ReplyMessage& reply) const;
    // Get the offers that a specific Nym has placed on a specific market.
    bool cmd_get_nymbox(ReplyMessage& reply) const;
    bool cmd_get_request_number(ReplyMessage& reply) const;
    bool cmd_get_transaction_numbers(ReplyMessage& reply) const;
    bool cmd_issue_basket(ReplyMessage& reply) const;
    bool cmd_notarize_transaction(ReplyMessage& reply) const;
    bool cmd_ping_notary(ReplyMessage& reply) const;
    bool cmd_process_inbox(ReplyMessage& reply) const;
    bool cmd_process_nymbox(ReplyMessage& reply) const;
    bool cmd_query_instrument_definitions(ReplyMessage& reply) const;
    bool cmd_register_account(ReplyMessage& reply) const;
    bool cmd_register_contract(ReplyMessage& reply) const;
    bool cmd_register_instrument_definition(ReplyMessage& reply) const;
    bool cmd_register_nym(ReplyMessage& reply) const;
    bool cmd_request_admin(ReplyMessage& reply) const;
    bool cmd_send_nym_message(ReplyMessage& reply) const;
    bool cmd_trigger_clause(ReplyMessage& reply) const;
    bool cmd_usage_credits(ReplyMessage& reply) const;
    std::unique_ptr<Ledger> create_nymbox(
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Nym& serverNym) const;
    bool hash_check(const ClientContext& context, Identifier& nymboxHash) const;
    RequestNumber initialize_request_number(ClientContext& context) const;
    std::unique_ptr<Ledger> load_inbox(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Identifier& serverID,
        const Nym& serverNym,
        const bool verifyAccount) const;
    std::unique_ptr<Ledger> load_nymbox(
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const Nym& serverNym,
        const bool verifyAccount) const;
    std::unique_ptr<Ledger> load_outbox(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Identifier& serverID,
        const Nym& serverNym,
        const bool verifyAccount) const;
    bool reregister_nym(ReplyMessage& reply) const;
    bool save_box(const Nym& nym, Ledger& box) const;
    bool save_inbox(const Nym& nym, Identifier& hash, Ledger& inbox) const;
    bool save_nymbox(const Nym& nym, Identifier& hash, Ledger& nymbox) const;
    bool save_outbox(const Nym& nym, Identifier& hash, Ledger& outbox) const;
    bool send_message_to_nym(
        const Identifier& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const Message& msg) const;
    bool verify_box(
        const Identifier& ownerID,
        Ledger& box,
        const Nym& nym,
        const bool full) const;
    bool verify_transaction(const OTTransaction* transaction, const Nym& signer)
        const;

    UserCommandProcessor(
        Server& server,
        const opentxs::api::server::Manager& manager);
    UserCommandProcessor() = delete;
    UserCommandProcessor(const UserCommandProcessor&) = delete;
    UserCommandProcessor(UserCommandProcessor&&) = delete;
    UserCommandProcessor& operator=(const UserCommandProcessor&) = delete;
    UserCommandProcessor& operator=(UserCommandProcessor&&) = delete;
};
}  // namespace server
}  // namespace opentxs
