// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/FaucetListener.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>

#include "internal/util/Future.hpp"
#include "internal/util/LogMacros.hpp"
#include "util/Work.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

class FaucetListener::Imp
{
public:
    using Message = opentxs::network::zeromq::Message;

    const ot::api::session::Client& api_;
    const ot::identifier::Nym local_nym_;

    auto GetFuture() noexcept
        -> std::shared_future<ot::blockchain::block::TransactionHash>
    {
        return future_;
    }
    auto Process(
        const opentxs::WorkType work,
        Message&& msg,
        ot::alloc::Strategy alloc) noexcept
        -> opentxs::network::zeromq::actor::Replies
    {
        using enum opentxs::WorkType;

        switch (work) {
            case PeerRequest: {
                process_request(std::move(msg));
            } break;
            default: {
            }
        }

        using namespace opentxs::network::zeromq::actor;
        auto out = Replies{alloc.work_};
        out.clear();
        auto& messages = out.emplace_back(
            LoopbackIndex,
            opentxs::Vector<opentxs::network::zeromq::Message>{alloc.work_});
        messages.second.emplace_back(
            opentxs::MakeWork(opentxs::OT_ZMQ_STATE_MACHINE_SIGNAL));

        return out;
    }
    auto StateMachine(ot::alloc::Strategy alloc) noexcept -> bool
    {
        if (finished_) { return false; }

        OT_ASSERT(request_.has_value());
        OT_ASSERT(remote_nym_);

        const auto& request = request_.value();
        const auto& nym = *remote_nym_;

        if (false == tx_.has_value()) {
            try {
                const auto pc = nym.PaymentCodePublic();

                OT_ASSERT(pc.Valid());

                const auto handle = api_.Network().Blockchain().GetChain(
                    opentxs::blockchain::Type::UnitTest);
                const auto& network = handle.get();
                tx_ = network.SendToPaymentCode(
                    local_nym_, pc, 1000000000, "faucet payment");

                return true;
            } catch (const std::exception& e) {
                opentxs::LogError()(__func__)(": ")(e.what()).Flush();
                promise_.set_value({});
                finished_ = true;

                return false;
            }
        } else if (false == otx_.has_value()) {
            if (opentxs::IsReady(*tx_)) {
                try {
                    const auto& txid = tx_->get().second;

                    if (txid.empty()) {

                        throw std::runtime_error{"invalid txid"};
                    } else {
                        txid_.emplace(txid);
                    }

                    const auto tx =
                        api_.Crypto().Blockchain().LoadTransaction(txid);

                    if (false == tx.IsValid()) {

                        throw std::runtime_error{"invalid tx"};
                    }

                    otx_ = api_.OTX().AcknowledgeFaucet(
                        request.Responder(),
                        request.Initiator(),
                        request.ID(),
                        tx);

                    return true;
                } catch (const std::exception& e) {
                    opentxs::LogError()(__func__)(": ")(e.what()).Flush();
                    promise_.set_value({});
                    finished_ = true;

                    return false;
                }
            } else {

                return true;
            }
        } else {
            if (opentxs::IsReady(otx_->second)) {
                try {
                    using enum opentxs::otx::LastReplyStatus;
                    const auto& [status, _] = otx_->second.get();

                    if (MessageSuccess == status) {
                        promise_.set_value(*txid_);
                    } else {
                        promise_.set_value({});
                    }

                    finished_ = true;

                    return false;
                } catch (const std::exception& e) {
                    opentxs::LogError()(__func__)(": ")(e.what()).Flush();
                    promise_.set_value({});
                    finished_ = true;

                    return false;
                }
            } else {

                return true;
            }
        }
    }

    Imp(const ot::api::session::Client& api,
        const ot::identifier::Nym& localNym,
        std::string_view name) noexcept
        : api_(api)
        , local_nym_(localNym)
        , promise_()
        , future_(promise_.get_future())
        , remote_nym_()
        , request_(std::nullopt)
        , tx_(std::nullopt)
        , txid_(std::nullopt)
        , otx_(std::nullopt)
        , finished_(false)
    {
    }

private:
    std::promise<ot::blockchain::block::TransactionHash> promise_;
    std::shared_future<ot::blockchain::block::TransactionHash> future_;
    opentxs::Nym_p remote_nym_;
    std::optional<opentxs::contract::peer::Request> request_;
    std::optional<opentxs::blockchain::node::Manager::PendingOutgoing> tx_;
    std::optional<ot::blockchain::block::TransactionHash> txid_;
    std::optional<opentxs::api::session::OTX::BackgroundTask> otx_;
    bool finished_;

    auto process_request(Message&& msg) noexcept -> void
    {
        if (request_.has_value()) { return; }

        try {
            const auto body = msg.Payload();
            const auto recipient =
                api_.Factory().NymIDFromProtobuf(body[2].Bytes());
            const auto sender =
                api_.Factory().NymIDFromProtobuf(body[3].Bytes());
            [[maybe_unused]] const auto type =
                body[4].as<opentxs::contract::peer::RequestType>();
            remote_nym_ = api_.Wallet().Nym(sender);

            OT_ASSERT(remote_nym_);

            request_.emplace(api_.Factory().PeerRequest(body[5]));
        } catch (const std::exception& e) {
            opentxs::LogError()(__func__)(": ")(e.what()).Flush();
            promise_.set_value({});
        }
    }
};
}  // namespace ottest

namespace ottest
{
FaucetListener::FaucetListener(
    const ot::api::session::Client& api,
    const ot::identifier::Nym& localNym,
    std::string_view name) noexcept
    : imp_(std::make_shared<Imp>(api, localNym, name))
{
    using namespace opentxs::network::zeromq;
    using enum socket::Direction;
    api.Network().ZeroMQ().SpawnActor(
        api,
        name,
        DefaultStartup(),
        DefaultShutdown(),
        [me = imp_](auto, auto type, auto&& message, auto alloc) {
            return me->Process(
                static_cast<opentxs::WorkType>(type),
                std::move(message),
                alloc);
        },
        [me = imp_](auto alloc) { return me->StateMachine(alloc); },
        {
            {api.Endpoints().Shutdown(), Connect},
            {api.Endpoints().PeerRequest(), Connect},
        });
}

auto FaucetListener::GetFuture() const noexcept
    -> std::shared_future<ot::blockchain::block::TransactionHash>
{
    return imp_->GetFuture();
}

FaucetListener::~FaucetListener() = default;
}  // namespace ottest
