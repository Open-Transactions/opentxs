// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <chrono>
#include <compare>
#include <ctime>
#include <future>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/client/Client.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/otx/client/obsolete/OT_API.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "otx/server/Server.hpp"
#include "otx/server/Transactor.hpp"

#define CHEQUE_AMOUNT 144488
#define TRANSFER_AMOUNT 1144888
#define SECOND_TRANSFER_AMOUNT 500000
#define CHEQUE_MEMO "cheque memo"
#define TRANSFER_MEMO "transfer memo"
#define SUCCESS true
#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT ot::UnitType::Usd
#define UNIT_DEFINITION_CONTRACT_NAME_2 "Mt Gox BTC"
#define UNIT_DEFINITION_TERMS_2 "YOLO"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT_2 ot::UnitType::Btc
#define MESSAGE_TEXT "example message text"
#define NEW_SERVER_NAME "Awesome McCoolName"
#define CASH_AMOUNT 100
#define MINT_TIME_LIMIT_MINUTES 5

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT Basic : public ::testing::Test
{
public:
    struct matchID {
        matchID(const ot::UnallocatedCString& id);
        auto operator()(
            const std::pair<ot::UnallocatedCString, ot::UnallocatedCString>& id)
            -> bool;
        const ot::UnallocatedCString id_;
    };

    static ot::RequestNumber alice_counter_;
    static ot::RequestNumber bob_counter_;
    static const ot::UnallocatedCString SeedA_;
    static const ot::UnallocatedCString SeedB_;
    static const ot::identifier::Nym alice_nym_id_;
    static const ot::identifier::Nym bob_nym_id_;
    static ot::TransactionNumber cheque_transaction_number_;
    static ot::UnallocatedCString bob_account_1_id_;
    static ot::UnallocatedCString bob_account_2_id_;
    static ot::UnallocatedCString outgoing_cheque_workflow_id_;
    static ot::UnallocatedCString incoming_cheque_workflow_id_;
    static ot::UnallocatedCString outgoing_transfer_workflow_id_;
    static ot::UnallocatedCString incoming_transfer_workflow_id_;
    static ot::UnallocatedCString internal_transfer_workflow_id_;
    static const ot::UnallocatedCString unit_id_1_;
    static const ot::UnallocatedCString unit_id_2_;
    static std::unique_ptr<ot::otx::client::internal::Operation>
        alice_state_machine_;
    static std::unique_ptr<ot::otx::client::internal::Operation>
        bob_state_machine_;
    static std::shared_ptr<ot::otx::blind::Purse> untrusted_purse_;

    const ot::api::session::Client& client_1_;
    const ot::api::session::Client& client_2_;
    const ot::api::session::Notary& server_1_;
    const ot::api::session::Notary& server_2_;
    ot::PasswordPrompt reason_c1_;
    ot::PasswordPrompt reason_c2_;
    ot::PasswordPrompt reason_s1_;
    ot::PasswordPrompt reason_s2_;
    const ot::OTUnitDefinition asset_contract_1_;
    const ot::OTUnitDefinition asset_contract_2_;
    const ot::identifier::Notary& server_1_id_;
    const ot::identifier::Notary& server_2_id_;
    const ot::OTServerContract server_contract_;
    const ot::otx::context::Server::ExtraArgs extra_args_;

    Basic();

    static auto find_id(
        const ot::UnallocatedCString& id,
        const ot::ObjectList& list) -> bool;
    static auto load_unit(
        const ot::api::Session& api,
        const ot::UnallocatedCString& id) noexcept -> ot::OTUnitDefinition;
    static auto translate_result(const ot::otx::LastReplyStatus status)
        -> ot::otx::client::SendResult;
    void break_consensus();
    void import_server_contract(
        const ot::contract::Server& contract,
        const ot::api::session::Client& client);
    void init();
    void create_unit_definition_1();
    void create_unit_definition_2();
    auto find_issuer_account() -> ot::identifier::Account;
    auto find_unit_definition_id_1() -> ot::identifier::UnitDefinition;
    auto find_unit_definition_id_2() -> ot::identifier::UnitDefinition;
    auto find_user_account() -> ot::identifier::Account;
    auto find_second_user_account() -> ot::identifier::Account;
    void receive_reply(
        const std::shared_ptr<const ot::identity::Nym>& recipient,
        const std::shared_ptr<const ot::identity::Nym>& sender,
        const ot::contract::peer::Reply& peerreply,
        const ot::contract::peer::Request& peerrequest,
        ot::contract::peer::RequestType requesttype);
    void receive_request(
        const std::shared_ptr<const ot::identity::Nym>& nym,
        const ot::contract::peer::Request& peerrequest,
        ot::contract::peer::RequestType requesttype);
    void send_peer_reply(
        const std::shared_ptr<const ot::identity::Nym>& nym,
        const ot::contract::peer::Reply& peerreply,
        const ot::contract::peer::Request& peerrequest,
        ot::contract::peer::RequestType requesttype);
    void send_peer_request(
        const std::shared_ptr<const ot::identity::Nym>& nym,
        const ot::contract::peer::Request& peerrequest,
        ot::contract::peer::RequestType requesttype);
    void verify_account(
        const ot::identity::Nym& clientNym,
        const ot::identity::Nym& serverNym,
        const ot::Account& client,
        const ot::Account& server,
        const ot::PasswordPrompt& reasonC,
        const ot::PasswordPrompt& reasonS);
    auto verify_bailment(
        const ot::contract::peer::Reply& originalreply,
        const ot::contract::peer::Reply& restoredreply) noexcept -> void;
    auto verify_bailment(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest) noexcept -> void;
    auto verify_bailment_notice(
        const ot::contract::peer::Reply& originalreply,
        const ot::contract::peer::Reply& restoredreply) noexcept -> void;
    auto verify_bailment_notice(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest) noexcept -> void;
    auto verify_connection(
        const ot::contract::peer::Reply& originalreply,
        const ot::contract::peer::Reply& restoredreply) noexcept -> void;
    auto verify_connection(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest) noexcept -> void;
    auto verify_outbailment(
        const ot::contract::peer::Reply& originalreply,
        const ot::contract::peer::Reply& restoredreply) noexcept -> void;
    auto verify_outbailment(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest) noexcept -> void;
    auto verify_reply(
        ot::contract::peer::RequestType requesttype,
        const ot::contract::peer::Reply& original,
        const ot::contract::peer::Reply& restored) noexcept -> void;
    auto verify_request(
        ot::contract::peer::RequestType requesttype,
        const ot::contract::peer::Request& original,
        const ot::contract::peer::Request& restored) noexcept -> void;
    template <typename T>
    void verify_reply_properties(
        const ot::contract::peer::Reply& original,
        const T& restored);
    template <typename T>
    void verify_request_properties(
        const ot::contract::peer::Request& original,
        const T& restored);
    void verify_storesecret(
        const ot::contract::peer::Request& originalrequest,
        const ot::contract::peer::Request& restoredrequest);
    void verify_state_pre(
        const ot::otx::context::Client& clientContext,
        const ot::otx::context::Server& serverContext,
        const ot::RequestNumber initialRequestNumber);
    void verify_state_post(
        const ot::api::session::Client& client,
        const ot::otx::context::Client& clientContext,
        const ot::otx::context::Server& serverContext,
        const ot::RequestNumber initialRequestNumber,
        const ot::RequestNumber finalRequestNumber,
        const ot::RequestNumber messageRequestNumber,
        const ot::otx::client::SendResult messageResult,
        const std::shared_ptr<ot::Message>& message,
        const bool messageSuccess,
        const std::size_t expectedNymboxItems,
        ot::RequestNumber& counter);
};

}  // namespace ottest
