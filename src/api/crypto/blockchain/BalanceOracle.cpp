// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "api/crypto/blockchain/BalanceOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <map>
#include <mutex>
#include <set>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::crypto::blockchain
{
struct BalanceOracle::Imp {
    auto RefreshBalance(const identifier::Nym& owner, const Chain chain)
        const noexcept -> void
    {
        try {
            const auto& network = api_.Network().Blockchain().GetChain(chain);
            UpdateBalance(chain, network.GetBalance());
            UpdateBalance(owner, chain, network.GetBalance(owner));
        } catch (...) {
        }
    }

    auto UpdateBalance(const Chain chain, const Balance balance) const noexcept
        -> void
    {
        const auto make = [&](auto& out, auto type) {
            out.AddFrame();
            out.AddFrame(value(type));
            out.AddFrame(chain);
            balance.first.Serialize(out.AppendBytes());
            balance.second.Serialize(out.AppendBytes());
        };
        {
            auto out = opentxs::network::zeromq::Message{};
            make(out, WorkType::BlockchainWalletUpdated);
            publisher_->Send(std::move(out));
        }
        const auto notify = [&](const auto& in) {
            auto out = opentxs::network::zeromq::Message{};
            out.AddFrame(in);
            make(out, WorkType::BlockchainBalance);
            socket_->Send(std::move(out));
        };
        {
            auto lock = Lock{lock_};
            const auto& subscribers = subscribers_[chain];
            std::for_each(
                std::begin(subscribers), std::end(subscribers), notify);
        }
    }

    auto UpdateBalance(
        const identifier::Nym& owner,
        const Chain chain,
        const Balance balance) const noexcept -> void
    {
        const auto make = [&](auto& out, auto type) {
            out.AddFrame();
            out.AddFrame(value(type));
            out.AddFrame(chain);
            balance.first.Serialize(out.AppendBytes());
            balance.second.Serialize(out.AppendBytes());
            out.AddFrame(owner);
        };
        {
            auto out = opentxs::network::zeromq::Message{};
            make(out, WorkType::BlockchainWalletUpdated);
            publisher_->Send(std::move(out));
        }
        const auto notify = [&](const auto& in) {
            auto out = opentxs::network::zeromq::Message{};
            out.AddFrame(in);
            make(out, WorkType::BlockchainBalance);
            socket_->Send(std::move(out));
        };
        {
            auto lock = Lock{lock_};
            const auto& subscribers = nym_subscribers_[chain][owner];
            std::for_each(
                std::begin(subscribers), std::end(subscribers), notify);
        }
    }

    Imp(const api::Session& api) noexcept
        : api_(api)
        , zmq_(api_.Network().ZeroMQ())
        , cb_(zmq::ListenCallback::Factory(
              [this](auto&& in) { cb(std::move(in)); }))
        , socket_([&] {
            auto out =
                zmq_.RouterSocket(cb_, zmq::socket::Socket::Direction::Bind);
            const auto started =
                out->Start(api_.Endpoints().BlockchainBalance());

            OT_ASSERT(started);

            return out;
        }())
        , publisher_([&] {
            auto out = zmq_.PublishSocket();
            const auto started =
                out->Start(api_.Endpoints().BlockchainWalletUpdated());

            OT_ASSERT(started);

            return out;
        }())
        , lock_()
        , subscribers_()
        , nym_subscribers_()
    {
    }

private:
    using Subscribers = std::pmr::set<OTData>;

    const api::Session& api_;
    const zmq::Context& zmq_;
    OTZMQListenCallback cb_;
    OTZMQRouterSocket socket_;
    OTZMQPublishSocket publisher_;
    mutable std::mutex lock_;
    mutable std::pmr::map<Chain, Subscribers> subscribers_;
    mutable std::pmr::map<Chain, std::pmr::map<OTNymID, Subscribers>>
        nym_subscribers_;

    auto cb(opentxs::network::zeromq::Message&& in) noexcept -> void
    {
        const auto header = in.Header();

        OT_ASSERT(0 < header.size());

        const auto& connectionID = header.at(0);
        const auto body = in.Body();

        OT_ASSERT(1 < body.size());

        const auto haveNym = (2 < body.size());
        auto output = opentxs::blockchain::Balance{};
        const auto& chainFrame = body.at(1);
        const auto nym = [&] {
            if (haveNym) {

                return api_.Factory().NymID(body.at(2));
            } else {

                return api_.Factory().NymID();
            }
        }();
        auto postcondition = ScopeGuard{[&]() {
            socket_->Send([&] {
                auto work = opentxs::network::zeromq::tagged_reply_to_message(
                    in, WorkType::BlockchainBalance);
                work.AddFrame(chainFrame);
                output.first.Serialize(work.AppendBytes());
                output.second.Serialize(work.AppendBytes());

                if (haveNym) { work.AddFrame(nym); }

                return work;
            }());
        }};
        const auto chain = chainFrame.as<Chain>();
        const auto unsupported =
            (0 == opentxs::blockchain::SupportedChains().count(chain)) &&
            (Chain::UnitTest != chain);

        if (unsupported) { return; }

        try {
            const auto& network = api_.Network().Blockchain().GetChain(chain);

            if (haveNym) {
                output = network.GetBalance(nym);
            } else {
                output = network.GetBalance();
            }

            auto lock = Lock{lock_};

            if (haveNym) {
                nym_subscribers_[chain][nym].emplace(
                    api_.Factory().Data(connectionID.Bytes()));
            } else {
                subscribers_[chain].emplace(
                    api_.Factory().Data(connectionID.Bytes()));
            }
        } catch (...) {
        }
    }
};

BalanceOracle::BalanceOracle(const api::Session& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
}

auto BalanceOracle::RefreshBalance(
    const identifier::Nym& owner,
    const Chain chain) const noexcept -> void
{
    imp_->RefreshBalance(owner, chain);
}

auto BalanceOracle::UpdateBalance(const Chain chain, const Balance balance)
    const noexcept -> void
{
    imp_->UpdateBalance(chain, balance);
}

auto BalanceOracle::UpdateBalance(
    const identifier::Nym& owner,
    const Chain chain,
    const Balance balance) const noexcept -> void
{
    imp_->UpdateBalance(owner, chain, balance);
}

BalanceOracle::~BalanceOracle() = default;
}  // namespace opentxs::api::crypto::blockchain
