// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "api/crypto/blockchain/BalanceOracle.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <chrono>
#include <string_view>
#include <utility>

#include "internal/core/Factory.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::api::crypto::blockchain
{
auto print(BalanceOracleJobs job) noexcept -> std::string_view
{
    try {
        using Type = BalanceOracleJobs;

        static const auto map = Map<Type, CString>{
            {Type::shutdown, "shutdown"},
            {Type::update_chain_balance, "update_chain_balance"},
            {Type::update_nym_balance, "update_nym_balance"},
            {Type::registration, "registration"},
            {Type::init, "init"},
            {Type::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid BalanceOracleJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::api::crypto::blockchain

namespace opentxs::api::crypto::blockchain
{
BalanceOracle::Imp::Imp(
    const api::Session& api,
    const opentxs::network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Actor(
          api,
          LogTrace(),
          0ms,
          batch,
          alloc,
          {
              {CString{api.Endpoints().Shutdown(), alloc}, Direction::Connect},
          },
          {},
          {},
          {
              {SocketType::Router,
               {
                   {CString{api.Endpoints().BlockchainBalance(), alloc},
                    Direction::Bind},
               }},
              {SocketType::Publish,
               {
                   {CString{api.Endpoints().BlockchainWalletUpdated(), alloc},
                    Direction::Bind},
               }},
          })
    , api_(api)
    , router_(pipeline_.Internal().ExtraSocket(0))
    , publish_(pipeline_.Internal().ExtraSocket(1))
    , data_(alloc)
{
}

auto BalanceOracle::Imp::make_message(
    const ReadView id,
    const identifier::Nym* owner,
    const Chain chain,
    const Balance& balance,
    const WorkType type) const noexcept -> Message
{
    auto out = Message{};

    if (valid(id)) { out.AddFrame(id.data(), id.size()); }

    out.StartBody();
    out.AddFrame(type);
    out.AddFrame(chain);
    out.AddFrame(balance.first);
    out.AddFrame(balance.second);

    if (nullptr != owner) { out.AddFrame(*owner); }

    return out;
}

auto BalanceOracle::Imp::notify_subscribers(
    const Subscribers& recipients,
    const Balance& balance,
    const Chain chain) noexcept -> void
{
    const auto& log = LogTrace();
    publish_.Send(make_message(
        {}, nullptr, chain, balance, WorkType::BlockchainWalletUpdated));

    for (const auto& id : recipients) {
        log(OT_PRETTY_CLASS())("notifying connection ")(id->asHex())(" for ")(
            print(chain))(" balance update");
        router_.Send(make_message(
            id->Bytes(), nullptr, chain, balance, WorkType::BlockchainBalance));
    }
}

auto BalanceOracle::Imp::notify_subscribers(
    const Subscribers& recipients,
    const identifier::Nym& owner,
    const Balance& balance,
    const Chain chain) noexcept -> void
{
    const auto& log = LogTrace();
    publish_.Send(make_message(
        {}, &owner, chain, balance, WorkType::BlockchainWalletUpdated));

    for (const auto& id : recipients) {
        log(OT_PRETTY_CLASS())("notifying connection ")(id->asHex())(" for ")(
            print(chain))(" balance update for nym ")(owner);
        router_.Send(make_message(
            id->Bytes(), &owner, chain, balance, WorkType::BlockchainBalance));
    }
}

auto BalanceOracle::Imp::pipeline(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::update_chain_balance: {
            process_update_chain_balance(std::move(msg));
        } break;
        case Work::update_nym_balance: {
            process_update_nym_balance(std::move(msg));
        } break;
        case Work::registration: {
            process_registration(std::move(msg));
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto BalanceOracle::Imp::process_registration(Message&& in) noexcept -> void
{
    const auto header = in.Header();

    OT_ASSERT(1 < header.size());

    // NOTE pipeline inserts an extra frame at the front of the message
    in.Internal().ExtractFront();
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
    const auto chain = chainFrame.as<Chain>();
    const auto postcondition = ScopeGuard{[&]() {
        router_.Send([&] {
            auto work = opentxs::network::zeromq::tagged_reply_to_message(
                in, WorkType::BlockchainBalance);
            work.AddFrame(chainFrame);
            output.first.Serialize(work.AppendBytes());
            output.second.Serialize(work.AppendBytes());

            if (haveNym) { work.AddFrame(nym); }

            return work;
        }());
    }};
    const auto unsupported =
        (0 == opentxs::blockchain::SupportedChains().count(chain)) &&
        (Chain::UnitTest != chain);

    if (unsupported) { return; }

    auto& subscribers = [&]() -> auto&
    {
        auto& chainData = data_[chain];

        if (haveNym) {

            return chainData.second[nym].second;
        } else {

            return chainData.first.second;
        }
    }
    ();
    const auto& id =
        subscribers.emplace(api_.Factory().Data(connectionID.Bytes()))
            .first->get();
    const auto& log = LogTrace();
    log(OT_PRETTY_CLASS())(id.asHex())(" subscribed to ")(print(chain))(
        " balance updates");

    if (haveNym) { log(" for nym ")(nym); }

    log.Flush();

    try {
        const auto& network = api_.Network().Blockchain().GetChain(chain);

        if (haveNym) {
            output = network.GetBalance(nym);
        } else {
            output = network.GetBalance();
        }
    } catch (...) {
    }
}

auto BalanceOracle::Imp::process_update_balance(
    const Chain chain,
    Balance input) noexcept -> void
{
    auto changed{false};
    auto& data = [&]() -> auto&
    {
        if (auto i = data_.find(chain); i != data_.end()) {
            auto& out = i->second.first;
            changed = (out.first != input);

            return out;
        } else {
            changed = true;

            return data_[chain].first;
        }
    }
    ();
    auto& balance = data.first;
    const auto& subscribers = data.second;
    balance.swap(input);

    if (changed) { notify_subscribers(subscribers, balance, chain); }
}

auto BalanceOracle::Imp::process_update_balance(
    const identifier::Nym& owner,
    const Chain chain,
    Balance input) noexcept -> void
{
    auto changed{false};
    auto& data = [&]() -> auto&
    {
        if (auto i = data_.find(chain); i != data_.end()) {
            auto& nymData = i->second.second;

            if (auto j = nymData.find(owner); j != nymData.end()) {
                auto& out = j->second;
                changed = (out.first != input);

                return out;
            } else {
                changed = true;

                return nymData[owner];
            }
        } else {
            changed = true;

            return data_[chain].second[owner];
        }
    }
    ();
    auto& balance = data.first;
    const auto& subscribers = data.second;
    balance.swap(input);

    if (changed) { notify_subscribers(subscribers, owner, balance, chain); }
}

auto BalanceOracle::Imp::process_update_chain_balance(Message&& in) noexcept
    -> void
{
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<Chain>();
    const auto balance = std::make_pair(
        factory::Amount(body.at(2)), factory::Amount(body.at(3)));
    process_update_balance(chain, balance);
}

auto BalanceOracle::Imp::process_update_nym_balance(Message&& in) noexcept
    -> void
{
    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<Chain>();
    const auto owner = api_.Factory().NymID(body.at(2));
    const auto balance = std::make_pair(
        factory::Amount(body.at(3)), factory::Amount(body.at(4)));
    process_update_balance(owner, chain, balance);
}

auto BalanceOracle::Imp::UpdateBalance(const Chain chain, const Balance balance)
    const noexcept -> void
{
    pipeline_.Push([&] {
        auto out = MakeWork(Work::update_chain_balance);
        out.AddFrame(chain);
        out.AddFrame(balance.first);
        out.AddFrame(balance.second);

        return out;
    }());
}

auto BalanceOracle::Imp::UpdateBalance(
    const identifier::Nym& owner,
    const Chain chain,
    const Balance balance) const noexcept -> void
{
    pipeline_.Push([&] {
        auto out = MakeWork(Work::update_nym_balance);
        out.AddFrame(chain);
        out.AddFrame(owner);
        out.AddFrame(balance.first);
        out.AddFrame(balance.second);

        return out;
    }());
}

auto BalanceOracle::Imp::work() noexcept -> bool { OT_FAIL; }

BalanceOracle::Imp::~Imp() { signal_shutdown(); }
}  // namespace opentxs::api::crypto::blockchain

namespace opentxs::api::crypto::blockchain
{
BalanceOracle::BalanceOracle(const api::Session& api) noexcept
    : imp_([&] {
        const auto& asio = api.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, api, batchID);
    }())
{
    OT_ASSERT(imp_);

    imp_->Init(imp_);
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

BalanceOracle::~BalanceOracle() { imp_->Shutdown(); }
}  // namespace opentxs::api::crypto::blockchain
