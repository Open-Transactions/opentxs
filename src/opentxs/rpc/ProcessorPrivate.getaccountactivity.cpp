// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/ProcessorPrivate.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PaymentWorkflow.pb.h>
#include <opentxs/protobuf/PaymentWorkflowEnums.pb.h>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/BalanceItem.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/otx/client/StorageBox.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/rpc/AccountEventType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/ResponseCode.hpp"      // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/request/GetAccountActivity.hpp"
#include "opentxs/rpc/request/Message.hpp"
#include "opentxs/rpc/response/GetAccountActivity.hpp"
#include "opentxs/rpc/response/Message.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::rpc
{
auto ProcessorPrivate::get_account_activity(const request::Message& base) const
    -> std::unique_ptr<response::Message>
{
    const auto& in = base.asGetAccountActivity();
    auto codes = response::Message::Responses{};
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

            const auto accountID = api.Factory().AccountIDFromBase58(id);
            const auto owner = [&]() -> identifier::Nym {
                const auto [chain, nym] =
                    api.Crypto().Blockchain().LookupAccount(accountID);

                if (nym.empty()) {

                    return api.Storage().Internal().AccountOwner(accountID);
                } else {

                    return nym;
                }
            }();
            // TODO check for empty owner and return appropriate error
            const auto& widget =
                api.UI().Internal().AccountActivity(owner, accountID);
            const auto copy = [&](const auto& row) {
                const auto contact = [&]() -> UnallocatedCString {
                    const auto& contacts = row.Contacts();

                    if (0u < contacts.size()) {
                        return contacts.front();
                    } else if (
                        otx::client::StorageBox::INTERNALTRANSFER ==
                        row.Type()) {

                        return api.Contacts().ContactID(owner).asBase58(
                            api.Crypto());
                    }

                    return {};
                }();
                const auto state = [&] {
                    auto out{protobuf::PAYMENTWORKFLOWSTATE_ERROR};
                    const auto wid =
                        api.Factory().IdentifierFromBase58(row.Workflow());

                    if (wid.empty()) { return out; }

                    auto proto = protobuf::PaymentWorkflow{};

                    if (api.Workflow().LoadWorkflow(owner, wid, proto)) {
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

auto ProcessorPrivate::get_account_event_type(
    otx::client::StorageBox storagebox,
    Amount amount) noexcept -> rpc::AccountEventType
{
    switch (storagebox) {
        case otx::client::StorageBox::INCOMINGCHEQUE: {

            return AccountEventType::incoming_cheque;
        }
        case otx::client::StorageBox::OUTGOINGCHEQUE: {

            return AccountEventType::outgoing_cheque;
        }
        case otx::client::StorageBox::INCOMINGTRANSFER: {

            return AccountEventType::incoming_transfer;
        }
        case otx::client::StorageBox::OUTGOINGTRANSFER: {

            return AccountEventType::outgoing_transfer;
        }
        case otx::client::StorageBox::INTERNALTRANSFER: {
            if (0 > amount) {

                return AccountEventType::outgoing_transfer;
            } else {

                return AccountEventType::incoming_transfer;
            }
        }
        case otx::client::StorageBox::BLOCKCHAIN: {
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
}  // namespace opentxs::rpc
