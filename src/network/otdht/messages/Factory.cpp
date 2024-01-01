// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/otdht/Factory.hpp"  // IWYU pragma: associated
#include "opentxs/network/otdht/Base.hpp"      // IWYU pragma: associated

#include <opentxs/protobuf/P2PBlockchainChainState.pb.h>
#include <opentxs/protobuf/P2PBlockchainHello.pb.h>
#include <opentxs/protobuf/P2PBlockchainSync.pb.h>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/WorkType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/network/otdht/Acknowledgement.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/PublishContract.hpp"
#include "opentxs/network/otdht/PublishContractReply.hpp"
#include "opentxs/network/otdht/PushTransaction.hpp"
#include "opentxs/network/otdht/PushTransactionReply.hpp"
#include "opentxs/network/otdht/Query.hpp"
#include "opentxs/network/otdht/QueryContract.hpp"
#include "opentxs/network/otdht/QueryContractReply.hpp"
#include "opentxs/network/otdht/Request.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/Types.internal.tpp"
#include "opentxs/protobuf/syntax/P2PBlockchainHello.hpp"
#include "opentxs/protobuf/syntax/P2PBlockchainSync.hpp"
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainSyncMessage(
    const api::Session& api,
    const network::zeromq::Message& in) noexcept
    -> std::unique_ptr<network::otdht::Base>
{
    const auto b = in.Payload();

    try {
        if (0 >= b.size()) {
            throw std::runtime_error{"missing message type frame"};
        }

        const auto& typeFrame = b[0];
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

                using MessageType = network::otdht::MessageType;
                const auto request = b[1].as<MessageType>();

                switch (request) {
                    case MessageType::publish_contract: {
                        if (3 >= b.size()) {
                            throw std::runtime_error{
                                "Insufficient frames (publish contract "
                                "response)"};
                        }

                        const auto& id = b[2];
                        const auto& success = b[3];

                        return BlockchainSyncPublishContractReply_p(
                            api, id.Bytes(), success.Bytes());
                    }
                    case MessageType::contract_query: {
                        if (4 >= b.size()) {
                            throw std::runtime_error{
                                "Insufficient frames (query contract "
                                "response)"};
                        }

                        const auto& contractType = b[2];
                        const auto& id = b[3];
                        const auto& payload = b[4];

                        return BlockchainSyncQueryContractReply_p(
                            api,
                            contractType.as<contract::Type>(),
                            id.Bytes(),
                            payload.Bytes());
                    }
                    case MessageType::pushtx: {
                        if (4 >= b.size()) {
                            throw std::runtime_error{
                                "Insufficient frames (pushtx response)"};
                        }

                        const auto& chain = b[2];
                        const auto& id = b[3];
                        const auto& success = b[4];

                        return BlockchainSyncPushTransactionReply_p(
                            api,
                            chain.as<opentxs::blockchain::Type>(),
                            id.Bytes(),
                            success.Bytes());
                    }
                    default: {
                        throw std::runtime_error{UnallocatedCString{
                            "unknown or invalid response type: "}
                                                     .append(print(request))};
                    }
                }
            }
            case WorkType::P2PPublishContract: {
                if (3 >= b.size()) {
                    throw std::runtime_error{
                        "Insufficient frames (publish contract)"};
                }

                const auto& contractType = b[1];
                const auto& id = b[2];
                const auto& payload = b[3];

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

                const auto& id = b[1];

                return BlockchainSyncQueryContract_p(api, id.Bytes());
            }
            case WorkType::P2PPushTransaction: {
                if (3 >= b.size()) {
                    throw std::runtime_error{
                        "Insufficient frames (publish contract)"};
                }

                const auto& chain = b[1];
                const auto& id = b[2];
                const auto& payload = b[3];

                return BlockchainSyncPushTransaction_p(
                    api,
                    chain.as<opentxs::blockchain::Type>(),
                    id.Bytes(),
                    payload.Bytes());
            }
            default: {
            }
        }

        if (1 >= b.size()) { throw std::runtime_error{"missing hello frame"}; }

        const auto& helloFrame = b[1];

        const auto hello =
            protobuf::Factory<protobuf::P2PBlockchainHello>(helloFrame);

        if (false == protobuf::syntax::check(LogError(), hello)) {
            throw std::runtime_error{"invalid hello"};
        }

        auto chains = [&] {
            // TODO allocator
            auto out = network::otdht::StateData{};

            for (const auto& state : hello.state()) {
                try {
                    out.emplace_back(
                        static_cast<opentxs::blockchain::Type>(state.chain()),
                        opentxs::blockchain::block::Position{
                            static_cast<opentxs::blockchain::block::Height>(
                                state.height()),
                            opentxs::blockchain::block::Hash{state.hash()}});
                } catch (const std::exception& e) {
                    // NOTE The remote peer might know about a newer blockchain
                    // type than what the local peer understands
                    LogTrace()()(e.what()).Flush();

                    continue;
                }
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

                const auto& cfheaderFrame = b[2];
                // TODO allocator
                auto data = network::otdht::SyncData{};
                using Chain = opentxs::blockchain::Type;
                auto chain = std::optional<Chain>{std::nullopt};
                using FilterType = opentxs::blockchain::cfilter::Type;
                auto filterType = std::optional<FilterType>{std::nullopt};
                using Height = opentxs::blockchain::block::Height;
                auto height = Height{-1};

                for (auto i{std::next(b.begin(), 3)}; i != b.end(); ++i) {
                    const auto sync =
                        protobuf::Factory<protobuf::P2PBlockchainSync>(*i);

                    if (false == protobuf::syntax::check(LogError(), sync)) {
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

                const auto& endpointFrame = b[2];

                return BlockchainSyncAcknowledgement_p(
                    std::move(chains),
                    UnallocatedCString{endpointFrame.Bytes()});
            }
            case WorkType::P2PBlockchainSyncRequest: {

                return BlockchainSyncRequest_p(std::move(chains));
            }
            case WorkType::P2PBlockchainSyncQuery:
            case WorkType::P2PResponse:
            case WorkType::P2PPublishContract:
            case WorkType::P2PQueryContract: {
                LogAbort()().Abort();
            }
            default: {
                throw std::runtime_error{"unsupported type"};
            }
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return std::make_unique<network::otdht::Base>();
    }
}
}  // namespace opentxs::factory
