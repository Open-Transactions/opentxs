// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <APIArgument.pb.h>
#include <AddClaim.pb.h>
#include <AddContact.pb.h>
#include <ContactItem.pb.h>
#include <CreateInstrumentDefinition.pb.h>
#include <CreateNym.pb.h>
#include <Enums.pb.h>
#include <GetWorkflow.pb.h>
#include <HDSeed.pb.h>
#include <ModifyAccount.pb.h>
#include <MoveFunds.pb.h>
#include <Nym.pb.h>
#include <PaymentWorkflow.pb.h>
#include <PaymentWorkflowEnums.pb.h>
#include <RPCCommand.pb.h>
#include <RPCEnums.pb.h>
#include <RPCResponse.pb.h>
#include <RPCStatus.pb.h>
#include <ServerContract.pb.h>
#include <SessionData.pb.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <utility>

#include "internal/otx/common/Account.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/RPCResponse.hpp"
#include "internal/util/Shared.hpp"
#include "ottest/Basic.hpp"
#include "ottest/fixtures/common/Base.hpp"

#define COMMAND_VERSION 3
#define RESPONSE_VERSION 3
#define STATUS_VERSION 2
#define APIARG_VERSION 1
#define CREATENYM_VERSION 2
#define ADDCONTACT_VERSION 1
#define CREATEINSTRUMENTDEFINITION_VERSION 1
#define MOVEFUNDS_VERSION 1
#define GETWORKFLOW_VERSION 1
#define SESSIONDATA_VERSION 1
#define MODIFYACCOUNT_VERSION 1
#define ADDCLAIM_VERSION 2
#define ADDCLAIM_SECTION_VERSION 6
#define CONTACTITEM_VERSION 6

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT Rpc : virtual public Base
{
public:
    Rpc() = default;

protected:
    static ot::identifier::UnitDefinition unit_definition_id_;
    static ot::UnallocatedCString issuer_account_id_;
    static ot::proto::ServerContract server_contract_;
    static ot::proto::ServerContract server2_contract_;
    static ot::proto::ServerContract server3_contract_;
    static ot::UnallocatedCString server_id_;
    static ot::UnallocatedCString server2_id_;
    static ot::UnallocatedCString server3_id_;
    static ot::UnallocatedCString nym1_id_;
    static ot::UnallocatedCString nym2_account_id_;
    static ot::UnallocatedCString nym2_id_;
    static ot::UnallocatedCString nym3_account1_id_;
    static ot::UnallocatedCString nym3_id_;
    static ot::UnallocatedCString nym3_account2_id_;
    static ot::UnallocatedCString seed_id_;
    static ot::UnallocatedCString seed2_id_;
    static ot::UnallocatedMap<ot::UnallocatedCString, int>
        widget_update_counters_;
    static ot::UnallocatedCString workflow_id_;
    static ot::UnallocatedCString claim_id_;

    static std::size_t get_index(const std::int32_t instance);
    static const ot::api::Session& get_session(const std::int32_t instance);

    proto::RPCCommand init(ot::proto::RPCCommandType commandtype);
    bool add_session(ot::proto::RPCCommandType commandtype, ArgList& args);
    void list(ot::proto::RPCCommandType commandtype, std::int32_t session = -1);

    void wait_for_state_machine(
        const ot::api::session::Client& api,
        const ot::identifier::Nym& nymID,
        const ot::identifier::Notary& serverID);
    void receive_payment(
        const ot::api::session::Client& api,
        const ot::identifier::Nym& nymID,
        const ot::identifier::Notary& serverID,
        const identifier::Account& accountID);
};

}  // namespace ottest
