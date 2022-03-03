// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "api/network/Dht.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>

#include "Proto.tpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/network/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/DhtConfig.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/ServerContract.pb.h"
#include "serialization/protobuf/UnitDefinition.pb.h"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto DhtAPI(
    const api::Session& api,
    const opentxs::network::zeromq::Context& zeromq,
    const api::session::Endpoints& endpoints,
    const bool defaultEnable,
    std::int64_t& nymPublishInterval,
    std::int64_t& nymRefreshInterval,
    std::int64_t& serverPublishInterval,
    std::int64_t& serverRefreshInterval,
    std::int64_t& unitPublishInterval,
    std::int64_t& unitRefreshInterval) noexcept
    -> std::unique_ptr<api::network::Dht>
{
    using ReturnType = api::network::imp::Dht;

    auto config = network::DhtConfig{};
    auto notUsed{false};
    api.Config().CheckSet_bool(
        String::Factory("OpenDHT"),
        String::Factory("enable_dht"),
        defaultEnable && (false == api.GetOptions().TestMode()),
        config.enable_dht_,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("nym_publish_interval"),
        config.nym_publish_interval_,
        nymPublishInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("nym_refresh_interval"),
        config.nym_refresh_interval_,
        nymRefreshInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("server_publish_interval"),
        config.server_publish_interval_,
        serverPublishInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("server_refresh_interval"),
        config.server_refresh_interval_,
        serverRefreshInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("unit_publish_interval"),
        config.unit_publish_interval_,
        unitPublishInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("unit_refresh_interval"),
        config.unit_refresh_interval_,
        unitRefreshInterval,
        notUsed);
    api.Config().CheckSet_long(
        String::Factory("OpenDHT"),
        String::Factory("listen_port"),
        config.default_port_ + api.Instance(),
        config.listen_port_,
        notUsed);
    api.Config().CheckSet_str(
        String::Factory("OpenDHT"),
        String::Factory("bootstrap_url"),
        String::Factory(config.bootstrap_url_),
        config.bootstrap_url_,
        notUsed);
    api.Config().CheckSet_str(
        String::Factory("OpenDHT"),
        String::Factory("bootstrap_port"),
        String::Factory(config.bootstrap_port_),
        config.bootstrap_port_,
        notUsed);

    return std::make_unique<ReturnType>(
        api, zeromq, endpoints, std::move(config));
}
}  // namespace opentxs::factory

namespace opentxs::api::network::imp
{
Dht::Dht(
    const api::Session& api,
    const opentxs::network::zeromq::Context& zeromq,
    const api::session::Endpoints& endpoints,
    opentxs::network::DhtConfig&& config) noexcept
    : api_(api)
    , config_(std::move(config))
    , lock_()
    , callback_map_()
    , node_(factory::OpenDHT(config_))
    , request_nym_callback_{zmq::ReplyCallback::Factory(
          [=](const zmq::Message& incoming)
              -> opentxs::network::zeromq::Message {
              return this->process_request(incoming, &Dht::GetPublicNym);
          })}
    , request_nym_socket_{zeromq.ReplySocket(
          request_nym_callback_,
          zmq::socket::Direction::Bind)}
    , request_server_callback_{zmq::ReplyCallback::Factory(
          [=](const zmq::Message& incoming)
              -> opentxs::network::zeromq::Message {
              return this->process_request(incoming, &Dht::GetServerContract);
          })}
    , request_server_socket_{zeromq.ReplySocket(
          request_server_callback_,
          zmq::socket::Direction::Bind)}
    , request_unit_callback_{zmq::ReplyCallback::Factory(
          [=](const zmq::Message& incoming)
              -> opentxs::network::zeromq::Message {
              return this->process_request(incoming, &Dht::GetUnitDefinition);
          })}
    , request_unit_socket_{zeromq.ReplySocket(
          request_unit_callback_,
          zmq::socket::Direction::Bind)}
{
    request_nym_socket_->Start(endpoints.DhtRequestNym().data());
    request_server_socket_->Start(endpoints.DhtRequestServer().data());
    request_unit_socket_->Start(endpoints.DhtRequestUnit().data());
}

auto Dht::Insert(const UnallocatedCString& key, const UnallocatedCString& value)
    const noexcept -> void
{
    node_->Insert(key, value);
}

auto Dht::Insert(const identity::Nym::Serialized& nym) const noexcept -> void
{
    node_->Insert(nym.nymid(), proto::ToString(nym));
}

auto Dht::Insert(const proto::ServerContract& contract) const noexcept -> void
{
    node_->Insert(contract.id(), proto::ToString(contract));
}

auto Dht::Insert(const proto::UnitDefinition& contract) const noexcept -> void
{
    node_->Insert(contract.id(), proto::ToString(contract));
}

auto Dht::GetPublicNym(const UnallocatedCString& key) const noexcept -> void
{
    auto lock = sLock{lock_};
    auto it = callback_map_.find(contract::Type::nym);
    bool haveCB = (it != callback_map_.end());
    NotifyCB notifyCB;

    if (haveCB) { notifyCB = it->second; }

    DhtResultsCallback gcb(
        [this, notifyCB, key](const DhtResults& values) -> bool {
            return ProcessPublicNym(api_, key, values, notifyCB);
        });

    node_->Retrieve(key, gcb);
}

auto Dht::GetServerContract(const UnallocatedCString& key) const noexcept
    -> void
{
    auto lock = sLock{lock_};
    auto it = callback_map_.find(contract::Type::notary);
    bool haveCB = (it != callback_map_.end());
    NotifyCB notifyCB;

    if (haveCB) { notifyCB = it->second; }

    DhtResultsCallback gcb(
        [this, notifyCB, key](const DhtResults& values) -> bool {
            return ProcessServerContract(api_, key, values, notifyCB);
        });

    node_->Retrieve(key, gcb);
}

auto Dht::GetUnitDefinition(const UnallocatedCString& key) const noexcept
    -> void
{
    auto lock = sLock{lock_};
    auto it = callback_map_.find(contract::Type::unit);
    bool haveCB = (it != callback_map_.end());
    NotifyCB notifyCB;

    if (haveCB) { notifyCB = it->second; }

    DhtResultsCallback gcb(
        [this, notifyCB, key](const DhtResults& values) -> bool {
            return ProcessUnitDefinition(api_, key, values, notifyCB);
        });

    node_->Retrieve(key, gcb);
}

auto Dht::OpenDHT() const noexcept -> const opentxs::network::OpenDHT&
{
    return *node_;
}

auto Dht::process_request(
    const zmq::Message& incoming,
    void (Dht::*get)(const UnallocatedCString&) const) const noexcept
    -> opentxs::network::zeromq::Message
{
    OT_ASSERT(nullptr != get)

    bool output{false};
    const auto body = incoming.Body();

    if (1 < body.size()) {
        const auto id = api_.Factory().Identifier(body.at(1));

        if (false == id->empty()) {
            output = true;
            (this->*get)(id->str());
        }
    }

    return [&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(output);

        return out;
    }();
}

auto Dht::ProcessPublicNym(
    const api::Session& api,
    const UnallocatedCString key,
    const DhtResults& values,
    NotifyCB notifyCB) noexcept -> bool
{
    UnallocatedCString theresult;
    bool foundData = false;
    bool foundValid = false;

    if (key.empty()) { return false; }

    for (const auto& it : values) {
        if (nullptr == it) { continue; }

        auto& data = *it;
        foundData = data.size() > 0;

        if (0 == data.size()) { continue; }

        auto publicNym = proto::Factory<proto::Nym>(data);

        if (key != publicNym.nymid()) { continue; }

        auto existing = api.Wallet().Nym(api.Factory().NymID(key));

        if (existing) {
            if (existing->Revision() >= publicNym.revision()) { continue; }
        }

        auto saved = api.Wallet().Internal().Nym(publicNym);

        if (!saved) { continue; }

        foundValid = true;

        LogDebug()(OT_PRETTY_STATIC(Dht))("Saved nym: ")(key).Flush();

        if (notifyCB) { notifyCB(key); }
    }

    if (!foundValid) {
        LogVerbose()(OT_PRETTY_STATIC(Dht))(
            "Found results, but none are valid.")
            .Flush();
    }

    if (!foundData) {
        LogVerbose()(OT_PRETTY_STATIC(Dht))("All results are empty.").Flush();
    }

    return foundData;
}

auto Dht::ProcessServerContract(
    const api::Session& api,
    const UnallocatedCString key,
    const DhtResults& values,
    NotifyCB notifyCB) noexcept -> bool
{
    UnallocatedCString theresult;
    bool foundData = false;
    bool foundValid = false;

    if (key.empty()) { return false; }

    for (const auto& it : values) {
        if (nullptr == it) { continue; }

        auto& data = *it;
        foundData = data.size() > 0;

        if (0 == data.size()) { continue; }

        auto contract = proto::Factory<proto::ServerContract>(data);

        if (key != contract.id()) { continue; }

        try {
            auto saved = api.Wallet().Internal().Server(contract);
        } catch (...) {
            continue;
        }

        LogDebug()(OT_PRETTY_STATIC(Dht))("Saved contract: ")(key).Flush();
        foundValid = true;

        if (notifyCB) { notifyCB(key); }

        break;  // We only need the first valid result
    }

    if (!foundValid) {
        LogError()(OT_PRETTY_STATIC(Dht))("Found results, but none are valid.")
            .Flush();
    }

    if (!foundData) {
        LogError()(OT_PRETTY_STATIC(Dht))("All results are empty.").Flush();
    }

    return foundData;
}

auto Dht::ProcessUnitDefinition(
    const api::Session& api,
    const UnallocatedCString key,
    const DhtResults& values,
    NotifyCB notifyCB) noexcept -> bool
{
    UnallocatedCString theresult;
    bool foundData = false;
    bool foundValid = false;

    if (key.empty()) { return false; }

    for (const auto& it : values) {
        if (nullptr == it) { continue; }

        auto& data = *it;
        foundData = data.size() > 0;

        if (0 == data.size()) { continue; }

        auto contract = proto::Factory<proto::UnitDefinition>(data);

        if (key != contract.id()) { continue; }

        try {
            api.Wallet().Internal().UnitDefinition(contract);
        } catch (...) {

            continue;
        }

        LogDebug()(OT_PRETTY_STATIC(Dht))("Saved unit definition: ")(key)
            .Flush();
        foundValid = true;

        if (notifyCB) { notifyCB(key); }

        break;  // We only need the first valid result
    }

    if (!foundValid) {
        LogError()(OT_PRETTY_STATIC(Dht))("Found results, but none are valid.")
            .Flush();
    }

    if (!foundData) {
        LogError()(OT_PRETTY_STATIC(Dht))("All results are empty.").Flush();
    }

    return foundData;
}

auto Dht::RegisterCallbacks(const CallbackMap& callbacks) const noexcept -> void
{
    auto lock = eLock{lock_};
    callback_map_ = callbacks;
}
}  // namespace opentxs::api::network::imp
