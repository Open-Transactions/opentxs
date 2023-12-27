// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/ProcessorPrivate.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Type.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Spend.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/rpc/PaymentType.hpp"   // IWYU pragma: keep
#include "opentxs/rpc/ResponseCode.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/request/Message.hpp"
#include "opentxs/rpc/request/SendPayment.hpp"
#include "opentxs/rpc/response/Message.hpp"
#include "opentxs/rpc/response/SendPayment.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::rpc
{
auto ProcessorPrivate::send_payment(const request::Message& base) const noexcept
    -> std::unique_ptr<response::Message>
{
    const auto& in = base.asSendPayment();
    const auto reply = [&](const auto code) {
        return std::make_unique<response::SendPayment>(
            in,
            response::Message::Responses{{0, code}},
            response::Message::Tasks{});
    };

    try {
        const auto& api = client_session(base);

        switch (in.PaymentType()) {
            case PaymentType::blockchain: {

                return send_payment_blockchain(api, in);
            }
            case PaymentType::cheque:
            case PaymentType::transfer:
            case PaymentType::voucher:
            case PaymentType::invoice:
            case PaymentType::blinded: {

                return send_payment_custodial(api, in);
            }
            case PaymentType::error:
            default: {

                return reply(ResponseCode::invalid);
            }
        }
    } catch (...) {

        return reply(ResponseCode::bad_session);
    }
}

auto ProcessorPrivate::send_payment_blockchain(
    const api::session::Client& api,
    const request::SendPayment& in) const noexcept
    -> std::unique_ptr<response::Message>
{
    auto tasks = response::Message::Tasks{};
    const auto reply = [&](const auto code) {
        return std::make_unique<response::SendPayment>(
            in, response::Message::Responses{{0, code}}, std::move(tasks));
    };

    const auto id = api.Factory().AccountIDFromBase58(in.SourceAccount());
    const auto& blockchain = api.Crypto().Blockchain();
    const auto data = blockchain.LookupAccount(id);
    const auto& [chain, owner] = data;

    if (blockchain::Type::UnknownBlockchain == chain) {
        return reply(ResponseCode::account_not_found);
    }

    api.Network().Blockchain().Start(chain);

    try {
        auto future = [&] {
            const auto& [chaintype, accountowner] = data;
            const auto amount = in.Amount();
            const auto& address = in.DestinationAccount();
            const auto& memo = in.Memo();
            const auto handle = api.Network().Blockchain().GetChain(chaintype);
            const auto recipient = api.Factory().PaymentCodeFromBase58(address);
            const auto& wallet = handle.get().Wallet();
            auto spend = wallet.CreateSpend(accountowner);

            if (false == spend.SetMemo(memo)) {
                throw std::runtime_error{"failed to set memo"};
            }

            if (0 < recipient.Version()) {
                if (false == spend.SendToPaymentCode(recipient, amount)) {
                    throw std::runtime_error{
                        "failed to set recipient payment code"};
                }
            } else {
                if (false == spend.SendToAddress(address, amount)) {
                    throw std::runtime_error{
                        "failed to set recipient payment code"};
                }
            }

            return wallet.Execute(spend);
        }();
        const auto [code, txid] = future.get();

        if (txid.empty()) { return reply(ResponseCode::transaction_failed); }

        tasks.emplace_back(0, txid.asHex());

        return reply(ResponseCode::txid);
    } catch (...) {

        return reply(ResponseCode::transaction_failed);
    }
}

auto ProcessorPrivate::send_payment_custodial(
    const api::session::Client& api,
    const request::SendPayment& command) const noexcept
    -> std::unique_ptr<response::Message>
{
    const auto contact =
        api.Factory().IdentifierFromBase58(command.RecipientContact());
    const auto source =
        api.Factory().AccountIDFromBase58(command.SourceAccount());
    auto tasks = response::Message::Tasks{};
    const auto reply = [&](const auto code) {
        return std::make_unique<response::SendPayment>(
            command, response::Message::Responses{{0, code}}, std::move(tasks));
    };

    if (contact.empty()) { return reply(ResponseCode::invalid); }

    if (auto c = api.Contacts().Contact(contact); false == bool(c)) {

        return reply(ResponseCode::contact_not_found);
    }

    const auto sender = api.Storage().Internal().AccountOwner(source);

    if (sender.empty()) { return reply(ResponseCode::account_owner_not_found); }

    const auto& otx = api.OTX();

    switch (otx.CanMessage(sender, contact)) {
        case otx::client::Messagability::MISSING_CONTACT:
        case otx::client::Messagability::CONTACT_LACKS_NYM:
        case otx::client::Messagability::NO_SERVER_CLAIM:
        case otx::client::Messagability::INVALID_SENDER:
        case otx::client::Messagability::MISSING_SENDER: {

            return reply(ResponseCode::no_path_to_recipient);
        }
        case otx::client::Messagability::MISSING_RECIPIENT:
        case otx::client::Messagability::UNREGISTERED: {

            return reply(ResponseCode::retry);
        }
        case otx::client::Messagability::READY:
        default: {
        }
    }

    switch (command.PaymentType()) {
        case PaymentType::cheque: {
            auto [taskID, future] = otx.SendCheque(
                sender, source, contact, command.Amount(), command.Memo());

            if (0 == taskID) { return reply(ResponseCode::error); }

            tasks.emplace_back(
                0,
                queue_task(
                    api,
                    sender,
                    std::to_string(taskID),
                    [&](const auto& in, auto& out) -> void {
                        evaluate_send_payment_cheque(in, out);
                    },
                    std::move(future)));

            return reply(ResponseCode::queued);
        }
        case PaymentType::transfer: {
            const auto destination =
                api.Factory().AccountIDFromBase58(command.DestinationAccount());
            const auto notary = api.Storage().Internal().AccountServer(source);
            auto [taskID, future] = otx.SendTransfer(
                sender,
                notary,
                source,
                destination,
                command.Amount(),
                command.Memo());

            if (0 == taskID) { return reply(ResponseCode::error); }

            tasks.emplace_back(
                0,
                queue_task(
                    api,
                    sender,
                    std::to_string(taskID),
                    [&](const auto& in, auto& out) -> void {
                        evaluate_send_payment_transfer(api, in, out);
                    },
                    std::move(future)));

            return reply(ResponseCode::queued);
        }
        case PaymentType::voucher:
        case PaymentType::invoice:
        case PaymentType::blinded:
        case PaymentType::error:
        case PaymentType::blockchain:
        default: {
            return std::make_unique<response::SendPayment>(
                command,
                response::Message::Responses{{0, ResponseCode::unimplemented}},
                response::Message::Tasks{});
        }
    }
}
}  // namespace opentxs::rpc
