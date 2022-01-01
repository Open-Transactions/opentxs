// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "opentxs/network/p2p/Base.hpp"  // IWYU pragma: associated

#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/network/p2p/Factory.hpp"
#include "internal/protobuf/Check.hpp"
#include "internal/protobuf/verify/BlockchainP2PHello.hpp"
#include "internal/protobuf/verify/BlockchainP2PSync.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/network/p2p/Acknowledgement.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/PublishContract.hpp"
#include "opentxs/network/p2p/PublishContractReply.hpp"
#include "opentxs/network/p2p/Query.hpp"
#include "opentxs/network/p2p/QueryContract.hpp"
#include "opentxs/network/p2p/QueryContractReply.hpp"
#include "opentxs/network/p2p/Request.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/p2p/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/BlockchainP2PChainState.pb.h"
#include "serialization/protobuf/BlockchainP2PHello.pb.h"
#include "serialization/protobuf/BlockchainP2PSync.pb.h"

namespace opentxs::factory
{
auto BlockchainSyncMessage(
    const api::Session& api,
    const network::zeromq::Message& in) noexcept
    -> std::unique_ptr<network::p2p::Base>
{
    const auto b = in.Body();

    try {
        if (0 >= b.size()) {
            throw std::runtime_error{"missing message type frame"};
        }

        const auto& typeFrame = b.at(0);
        const auto work = [&] {
            try {

                return typeFrame.as<WorkType>();
            } catch (...) {

                throw std::runtime_error{"Invalid type"};
            }
        }();

        switch (work) {
            case WorkType::P2PBlockchainSyncQuery: {

                return BlockchainSyncQuery_p(0);
            }
            case WorkType::P2PResponse: {
                if (1 >= b.size()) {
                    throw std::runtime_error{"missing response type frame"};
                }

                using MessageType = network::p2p::MessageType;
                const auto request = b.at(1).as<MessageType>();

                switch (request) {
                    case MessageType::publish_contract: {
                        if (3 >= b.size()) {
                            throw std::runtime_error{
                                "Insufficient frames (publish contract "
                                "response)"};
                        }

                        const auto& id = b.at(2);
                        const auto& success = b.at(3);

                        return BlockchainSyncPublishContractReply_p(
                            api, id.Bytes(), success.Bytes());
                    }
                    case MessageType::contract_query: {
                        if (4 >= b.size()) {
                            throw std::runtime_error{
                                "Insufficient frames (query contract "
                                "response)"};
                        }

                        const auto& contractType = b.at(2);
                        const auto& id = b.at(3);
                        const auto& payload = b.at(4);

                        return BlockchainSyncQueryContractReply_p(
                            api,
                            contractType.as<contract::Type>(),
                            id.Bytes(),
                            payload.Bytes());
                    }
                    default: {
                        throw std::runtime_error{
                            std::string{"unknown or invalid response type: "} +
                            opentxs::print(request)};
                    }
                }
            }
            case WorkType::P2PPublishContract: {
                if (3 >= b.size()) {
                    throw std::runtime_error{
                        "Insufficient frames (publish contract)"};
                }

                const auto& contractType = b.at(1);
                const auto& id = b.at(2);
                const auto& payload = b.at(3);

                return BlockchainSyncPublishContract_p(
                    api,
                    contractType.as<contract::Type>(),
                    id.Bytes(),
                    payload.Bytes());
            }
            case WorkType::P2PQueryContract: {
                if (1 >= b.size()) {
                    throw std::runtime_error{
                        "Insufficient frames (query contract)"};
                }

                const auto& id = b.at(1);

                return BlockchainSyncQueryContract_p(api, id.Bytes());
            }
            default: {
            }
        }

        if (1 >= b.size()) { throw std::runtime_error{"missing hello frame"}; }

        const auto& helloFrame = b.at(1);

        const auto hello =
            proto::Factory<proto::BlockchainP2PHello>(helloFrame);

        if (false == proto::Validate(hello, VERBOSE)) {
            throw std::runtime_error{"invalid hello"};
        }

        auto chains = [&] {
            auto out = std::pmr::vector<network::p2p::State>{};

            for (const auto& state : hello.state()) {
                auto hash = [&] {
                    auto out = api.Factory().Data();
                    out->Assign(state.hash().data(), state.hash().size());

                    return out;
                }();
                out.emplace_back(network::p2p::State{
                    static_cast<opentxs::blockchain::Type>(state.chain()),
                    {state.height(), std::move(hash)}});
            }

            return out;
        }();

        switch (work) {
            case WorkType::P2PBlockchainNewBlock:
            case WorkType::P2PBlockchainSyncReply: {
                if (3 > b.size()) {
                    throw std::runtime_error{
                        "insufficient frames (block data)"};
                }

                const auto& cfheaderFrame = b.at(2);
                auto data = std::pmr::vector<network::p2p::Block>{};
                using Chain = opentxs::blockchain::Type;
                auto chain = std::optional<Chain>{std::nullopt};
                using FilterType = opentxs::blockchain::filter::Type;
                auto filterType = std::optional<FilterType>{std::nullopt};
                using Height = opentxs::blockchain::block::Height;
                auto height = Height{-1};

                for (auto i{std::next(b.begin(), 3)}; i != b.end(); ++i) {
                    const auto sync =
                        proto::Factory<proto::BlockchainP2PSync>(*i);

                    if (false == proto::Validate(sync, VERBOSE)) {
                        throw std::runtime_error{"invalid sync data"};
                    }

                    const auto incomingChain = static_cast<Chain>(sync.chain());
                    const auto incomingHeight =
                        static_cast<Height>(sync.height());
                    const auto incomingType =
                        static_cast<FilterType>(sync.filter_type());

                    if (chain.has_value()) {
                        if (incomingHeight != ++height) {
                            throw std::runtime_error{
                                "non-contiguous sync data"};
                        }

                        if (incomingChain != chain.value()) {
                            throw std::runtime_error{"incorrect chain"};
                        }

                        if (incomingType != filterType.value()) {
                            throw std::runtime_error{"incorrect filter type"};
                        }
                    } else {
                        chain = incomingChain;
                        height = incomingHeight;
                        filterType = incomingType;
                    }

                    data.emplace_back(sync);
                }

                if (0 == chains.size()) {
                    throw std::runtime_error{"missing state"};
                }

                return BlockchainSyncData_p(
                    work,
                    std::move(chains.front()),
                    std::move(data),
                    cfheaderFrame.Bytes());
            }
            case WorkType::P2PBlockchainSyncAck: {
                if (2 >= b.size()) {
                    throw std::runtime_error{"missing endpoint frame"};
                }

                const auto& endpointFrame = b.at(2);

                return BlockchainSyncAcknowledgement_p(
                    std::move(chains), std::string{endpointFrame.Bytes()});
            }
            case WorkType::P2PBlockchainSyncRequest: {

                return BlockchainSyncRequest_p(std::move(chains));
            }
            case WorkType::P2PBlockchainSyncQuery:
            case WorkType::P2PResponse:
            case WorkType::P2PPublishContract:
            case WorkType::P2PQueryContract: {
                OT_FAIL;
            }
            default: {
                throw std::runtime_error{"unsupported type"};
            }
        }
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return std::make_unique<network::p2p::Base>();
    }
}
}  // namespace opentxs::factory
