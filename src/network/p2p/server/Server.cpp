// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "network/p2p/server/Server.hpp"  // IWYU pragma: associated

#include <atomic>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/network/p2p/Factory.hpp"
#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Thread.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/p2p/Acknowledgement.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/PublishContract.hpp"
#include "opentxs/network/p2p/PublishContractReply.hpp"
#include "opentxs/network/p2p/QueryContract.hpp"
#include "opentxs/network/p2p/QueryContractReply.hpp"
#include "opentxs/network/p2p/Request.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/p2p/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "util/Gatekeeper.hpp"

namespace opentxs::network::p2p
{
class Server::Imp
{
public:
    using Map = UnallocatedMap<
        Chain,
        std::tuple<
            UnallocatedCString,
            bool,
            std::reference_wrapper<zeromq::socket::Raw>>>;

    const api::Session& api_;
    const zeromq::Context& zmq_;
    zeromq::internal::Batch& batch_;
    const zeromq::ListenCallback& external_callback_;
    const zeromq::ListenCallback& internal_callback_;
    zeromq::socket::Raw& sync_;
    zeromq::socket::Raw& update_;
    mutable std::mutex map_lock_;
    Map map_;
    zeromq::internal::Thread* thread_;
    UnallocatedCString sync_endpoint_;
    UnallocatedCString sync_public_endpoint_;
    UnallocatedCString update_endpoint_;
    UnallocatedCString update_public_endpoint_;
    std::atomic_bool started_;
    std::atomic_bool running_;
    mutable Gatekeeper gate_;

    Imp(const api::Session& api, const zeromq::Context& zmq) noexcept
        : api_(api)
        , zmq_(zmq)
        , batch_(zmq_.Internal().MakeBatch([&] {
            auto out = UnallocatedVector<zeromq::socket::Type>{};
            out.emplace_back(zeromq::socket::Type::Router);
            out.emplace_back(zeromq::socket::Type::Publish);
            const auto chains = opentxs::blockchain::DefinedChains().size();
            out.insert(out.end(), chains, zeromq::socket::Type::Pair);

            return out;
        }()))
        , external_callback_(batch_.listen_callbacks_.emplace_back(
              zeromq::ListenCallback::Factory(
                  [this](auto&& msg) { process_external(std::move(msg)); })))
        , internal_callback_(batch_.listen_callbacks_.emplace_back(
              zeromq::ListenCallback::Factory(
                  [this](auto&& msg) { process_internal(std::move(msg)); })))
        , sync_(batch_.sockets_.at(0))
        , update_(batch_.sockets_.at(1))
        , map_lock_()
        , map_([&] {
            auto output = Map{};

            OT_ASSERT(
                batch_.sockets_.size() ==
                opentxs::blockchain::DefinedChains().size() + 2u);

            auto it = std::next(batch_.sockets_.begin(), 1);

            for (const auto chain : opentxs::blockchain::DefinedChains()) {
                auto& [endpoint, enabled, socket] =
                    output
                        .emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(chain),
                            std::forward_as_tuple(
                                opentxs::network::zeromq::MakeArbitraryInproc(),
                                false,
                                *(++it)))
                        .first->second;
                socket.get().Bind(endpoint.c_str());
                LogTrace()(OT_PRETTY_CLASS())("socket ")(socket.get().ID())(
                    " for ")(DisplayString(chain))(" bound to ")(endpoint)
                    .Flush();
            }

            return output;
        }())
        , thread_(zmq_.Internal().Start(
              batch_.id_,
              [&] {
                  auto out = zeromq::StartArgs{};
                  out.reserve(map_.size() + 1u);
                  out.emplace_back(
                      sync_.ID(), &sync_, [&cb = external_callback_](auto&& m) {
                          cb.Process(std::move(m));
                      });

                  for (auto& [chain, data] : map_) {
                      auto& [endpoint, enabled, wrapper] = data;
                      auto& socket = wrapper.get();

                      out.emplace_back(
                          socket.ID(),
                          &socket,
                          [&cb = internal_callback_](auto&& m) {
                              cb.Process(std::move(m));
                          });
                  }

                  return out;
              }()))
        , sync_endpoint_()
        , sync_public_endpoint_()
        , update_endpoint_()
        , update_public_endpoint_()
        , started_(false)
        , running_(true)
        , gate_()
    {
    }

    ~Imp()
    {
        gate_.shutdown();

        if (auto running = running_.exchange(false); running) {
            batch_.ClearCallbacks();
        }

        zmq_.Internal().Stop(batch_.id_);
    }

private:
    auto process_external(zeromq::Message&& incoming) noexcept -> void
    {
#if OT_BLOCKCHAIN
        try {
            namespace bcsync = opentxs::network::p2p;
            const auto base = api_.Factory().BlockchainSyncMessage(incoming);

            if (!base) {
                throw std::runtime_error{"failed to instantiate message"};
            }

            const auto type = base->Type();

            switch (type) {
                case bcsync::MessageType::query:
                case bcsync::MessageType::sync_request: {
                    process_sync(std::move(incoming), *base);
                } break;
                case bcsync::MessageType::contract_query: {
                    process_query_contract(std::move(incoming), *base);
                } break;
                case bcsync::MessageType::publish_contract: {
                    process_publish_contract(std::move(incoming), *base);
                } break;
                default: {
                    throw std::runtime_error{
                        UnallocatedCString{"Unsupported message type "} +
                        opentxs::print(type)};
                }
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        }
#endif  // OT_BLOCKCHAIN
    }
    auto process_internal(zeromq::Message&& incoming) noexcept -> void
    {
        const auto hSize = incoming.Header().size();
        const auto bSize = incoming.Body().size();

        if ((0u == hSize) && (0u == bSize)) { return; }

        if (0u < hSize) {
            LogTrace()(OT_PRETTY_CLASS())("transmitting sync reply").Flush();
            sync_.Send(std::move(incoming));
        } else {
            LogTrace()(OT_PRETTY_CLASS())("broadcasting push notification")
                .Flush();
            update_.Send(std::move(incoming));
        }
    }
    auto process_query_contract(
        opentxs::network::zeromq::Message&& incoming,
        const opentxs::network::p2p::Base& base) noexcept -> void
    {
        auto payload = [&] {
            const auto& id = base.asQueryContract().ID();

            try {
                const auto type = translate(id.Type());

                switch (type) {
                    case contract::Type::nym: {
                        const auto nymID =
                            api_.Factory().InternalSession().NymID(id);
                        const auto nym = api_.Wallet().Nym(nymID);

                        if (!nym) {
                            throw std::runtime_error{
                                UnallocatedCString{"nym "} + nymID->str() +
                                " not found"};
                        }

                        return factory::BlockchainSyncQueryContractReply(*nym);
                    }
                    case contract::Type::notary: {
                        const auto notaryID =
                            api_.Factory().InternalSession().ServerID(id);

                        return factory::BlockchainSyncQueryContractReply(
                            api_.Wallet().Server(notaryID));
                    }
                    case contract::Type::unit: {
                        const auto unitID =
                            api_.Factory().InternalSession().UnitID(id);

                        return factory::BlockchainSyncQueryContractReply(
                            api_.Wallet().UnitDefinition(unitID));
                    }
                    default: {
                        throw std::runtime_error{
                            UnallocatedCString{
                                "unsupported or unknown contract type: "} +
                            opentxs::print(type)};
                    }
                }
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

                return factory::BlockchainSyncQueryContractReply(id);
            }
        }();
        sync_.Send([&] {
            auto out =
                opentxs::network::zeromq::reply_to_message(std::move(incoming));
            payload.Serialize(out);

            return out;
        }());
    }
    auto process_publish_contract(
        opentxs::network::zeromq::Message&& incoming,
        const opentxs::network::p2p::Base& base) noexcept -> void
    {
        const auto& contract = base.asPublishContract();
        const auto& id = contract.ID();
        auto payload = [&] {
            try {
                const auto type = contract.ContractType();

                switch (type) {
                    case contract::Type::nym: {
                        const auto nym = api_.Wallet().Nym(contract.Payload());

                        return (nym && nym->ID() == id);
                    }
                    case contract::Type::notary: {
                        const auto notary =
                            api_.Wallet().Server(contract.Payload());

                        return (notary->ID() == id);
                    }
                    case contract::Type::unit: {
                        const auto unit =
                            api_.Wallet().UnitDefinition(contract.Payload());

                        return (unit->ID() == id);
                    }
                    default: {
                        throw std::runtime_error{
                            UnallocatedCString{
                                "unsupported or unknown contract type: "} +
                            opentxs::print(type)};
                    }
                }
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

                return false;
            }
        }();
        sync_.Send([&] {
            auto out =
                opentxs::network::zeromq::reply_to_message(std::move(incoming));
            const auto msg =
                factory::BlockchainSyncPublishContractReply(id, payload);
            msg.Serialize(out);

            return out;
        }());
    }
    auto process_sync(
        opentxs::network::zeromq::Message&& incoming,
        const opentxs::network::p2p::Base& base) noexcept -> void
    {
        try {
            {
                const auto ack = [&] {
                    auto lock = Lock{map_lock_};

                    return factory::BlockchainSyncAcknowledgement(
                        api_.Network().Blockchain().Internal().Hello(),
                        update_public_endpoint_);
                }();
                auto msg = opentxs::network::zeromq::reply_to_message(incoming);

                if (ack.Serialize(msg)) { sync_.Send(std::move(msg)); }
            }

            namespace bcsync = opentxs::network::p2p;
            const auto type = base.Type();

            if (bcsync::MessageType::sync_request == type) {
                const auto& request = base.asRequest();

                for (const auto& state : request.State()) {
                    try {
                        auto lock = Lock{map_lock_};
                        const auto& [endpoint, enabled, socket] =
                            map_.at(state.Chain());

                        if (enabled) { socket.get().Send(std::move(incoming)); }
                    } catch (...) {
                    }
                }
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        }
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Server::Server(const api::Session& api, const zeromq::Context& zmq) noexcept
    : imp_(std::make_unique<Imp>(api, zmq))
{
}

auto Server::Disable(const Chain chain) noexcept -> void
{
    try {
        auto lock = Lock{imp_->map_lock_};
        std::get<1>(imp_->map_.at(chain)) = false;
    } catch (...) {
    }
}

auto Server::Enable(const Chain chain) noexcept -> void
{
    try {
        auto lock = Lock{imp_->map_lock_};
        std::get<1>(imp_->map_.at(chain)) = true;
    } catch (...) {
    }
}

auto Server::Endpoint(const Chain chain) const noexcept -> UnallocatedCString
{
    try {

        return std::get<0>(imp_->map_.at(chain));
    } catch (...) {

        return {};
    }
}

auto Server::Start(
    const UnallocatedCString& sync,
    const UnallocatedCString& publicSync,
    const UnallocatedCString& update,
    const UnallocatedCString& publicUpdate) noexcept -> bool
{
    if (sync.empty() || update.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid endpoint").Flush();

        return false;
    }

    const auto shutdown = imp_->gate_.get();

    if (shutdown) { return false; }

    if (auto started = imp_->started_.exchange(true); started) {
        LogError()(OT_PRETTY_CLASS())("Already running").Flush();

        return false;
    } else {
        auto lock = Lock{imp_->map_lock_};
        imp_->sync_endpoint_ = sync;
        imp_->sync_public_endpoint_ = publicSync;
        imp_->update_endpoint_ = update;
        imp_->update_public_endpoint_ = publicUpdate;
    }

    imp_->thread_->Modify(imp_->sync_.ID(), [this](auto& socket) {
        const auto& endpointPublic = imp_->sync_public_endpoint_;
        const auto& endpointLocal = imp_->sync_endpoint_;
        const auto& endpointPublish = imp_->update_endpoint_;

        if (socket.SetRoutingID(endpointPublic)) {
            LogTrace()("Sync socket identity set to public endpoint: ")(
                endpointPublic)
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())("failed to set sync socket identity")
                .Flush();
        }

        if (socket.Bind(endpointLocal.c_str())) {
            LogConsole()("Blockchain sync server listener bound to ")(
                endpointLocal)
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())("failed to bind sync endpoint to ")(
                endpointLocal)
                .Flush();
        }

        if (imp_->update_.Bind(endpointPublish.c_str())) {
            LogConsole()("Blockchain sync publisher listener bound to ")(
                endpointPublish)
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())("failed to bind sync publisher to ")(
                endpointPublish)
                .Flush();
        }
    });

    return true;
}

Server::~Server() = default;
}  // namespace opentxs::network::p2p
