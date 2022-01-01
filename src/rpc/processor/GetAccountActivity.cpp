// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "rpc/RPC.tpp"     // IWYU pragma: associated

#include <string>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/rpc/AccountEventType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/GetAccountActivity.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/GetAccountActivity.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "rpc/RPC.hpp"
#include "serialization/protobuf/PaymentWorkflow.pb.h"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"

namespace opentxs::rpc::implementation
{
auto RPC::get_account_activity(const request::Base& base) const
    -> std::unique_ptr<response::Base>
{
    const auto& in = base.asGetAccountActivity();
    auto codes = response::Base::Responses{};
    auto events = response::GetAccountActivity::Events{};
    const auto reply = [&] {
        return std::make_unique<response::GetAccountActivity>(
            in, std::move(codes), std::move(events));
    };

    try {
        const auto& api = client_session(base);

        for (const auto& id : in.Accounts()) {
            const auto index = codes.size();

            if (id.empty()) {
                codes.emplace_back(index, ResponseCode::invalid);

                continue;
            }

            const auto accountID = api.Factory().IdentifierFromBase58(id);
            const auto owner = [&]() -> OTNymID {
                const auto [chain, owner] =
                    api.Crypto().Blockchain().LookupAccount(accountID);

                if (owner->empty()) {

                    return api.Storage().AccountOwner(accountID);
                } else {

                    return owner;
                }
            }();
            // TODO check for empty owner and return appropriate error
            const auto& widget = api.UI().AccountActivity(owner, accountID);
            const auto copy = [&](const auto& row) {
                const auto contact = [&]() -> std::string {
                    const auto& contacts = row.Contacts();

                    if (0u < contacts.size()) {
                        return contacts.front();
                    } else if (StorageBox::INTERNALTRANSFER == row.Type()) {

                        return api.Contacts().ContactID(owner)->str();
                    }

                    return {};
                }();
                const auto state = [&] {
                    auto out{proto::PAYMENTWORKFLOWSTATE_ERROR};
                    const auto id =
                        api.Factory().IdentifierFromBase58(row.Workflow());

                    if (id->empty()) { return out; }

                    auto proto = proto::PaymentWorkflow{};

                    if (api.Workflow().LoadWorkflow(owner, id, proto)) {
                        return proto.state();
                    }

                    return out;
                }();
                events.emplace_back(
                    id,
                    get_account_event_type(row.Type(), row.Amount()),
                    contact,
                    row.Workflow(),
                    row.DisplayAmount(),
                    row.DisplayAmount(),
                    row.Amount(),
                    row.Amount(),
                    row.Timestamp(),
                    row.Memo(),
                    row.UUID(),
                    state);
            };
            auto row = widget.First();

            if (false == row->Valid()) {
                codes.emplace_back(index, ResponseCode::none);

                continue;
            }

            copy(row.get());

            while (false == row->Last()) {
                row = widget.Next();
                copy(row.get());
            }

            codes.emplace_back(index, ResponseCode::success);
        }
    } catch (...) {
        codes.emplace_back(0, ResponseCode::bad_session);
    }

    return reply();
}

auto RPC::get_account_event_type(StorageBox storagebox, Amount amount) noexcept
    -> rpc::AccountEventType
{
    switch (storagebox) {
        case StorageBox::INCOMINGCHEQUE: {

            return AccountEventType::incoming_cheque;
        }
        case StorageBox::OUTGOINGCHEQUE: {

            return AccountEventType::outgoing_cheque;
        }
        case StorageBox::INCOMINGTRANSFER: {

            return AccountEventType::incoming_transfer;
        }
        case StorageBox::OUTGOINGTRANSFER: {

            return AccountEventType::outgoing_transfer;
        }
        case StorageBox::INTERNALTRANSFER: {
            if (0 > amount) {

                return AccountEventType::outgoing_transfer;
            } else {

                return AccountEventType::incoming_transfer;
            }
        }
        case StorageBox::BLOCKCHAIN: {
            if (0 > amount) {

                return AccountEventType::outgoing_blockchain;
            } else {

                return AccountEventType::incoming_blockchain;
            }
        }
        default: {

            return AccountEventType::error;
        }
    }
}
}  // namespace opentxs::rpc::implementation
