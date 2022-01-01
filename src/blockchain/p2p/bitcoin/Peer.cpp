// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/Peer.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <array>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "blockchain/DownloadTask.hpp"
#include "blockchain/bitcoin/Inventory.hpp"
#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/message/Cmpctblock.hpp"
#include "blockchain/p2p/bitcoin/message/Feefilter.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocks.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocktxn.hpp"
#include "blockchain/p2p/bitcoin/message/Merkleblock.hpp"
#include "blockchain/p2p/bitcoin/message/Reject.hpp"
#include "blockchain/p2p/bitcoin/message/Sendcmpct.hpp"
#include "blockchain/p2p/peer/Address.hpp"
#include "blockchain/p2p/peer/ConnectionManager.hpp"
#include "blockchain/p2p/peer/Peer.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Factory.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/p2p/Peer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"  // IWYU pragma: keep
#include "util/ScopeGuard.hpp"

namespace opentxs::factory
{
auto BitcoinP2PPeerLegacy(
    const api::Session& api,
    const blockchain::node::internal::Config& config,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::node::internal::Network& network,
    const blockchain::node::HeaderOracle& header,
    const blockchain::node::internal::FilterOracle& filter,
    const blockchain::node::internal::BlockOracle& block,
    const blockchain::node::internal::PeerManager& manager,
    const blockchain::database::BlockStorage policy,
    const int id,
    std::unique_ptr<blockchain::p2p::internal::Address> address,
    const std::string& shutdown)
    -> std::unique_ptr<blockchain::p2p::internal::Peer>
{
    namespace p2p = blockchain::p2p;
    using ReturnType = p2p::bitcoin::implementation::Peer;

    if (false == bool(address)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid null address")
            .Flush();

        return {};
    }

    switch (address->Type()) {
        case p2p::Network::ipv6:
        case p2p::Network::ipv4:
        case p2p::Network::zmq: {
            break;
        }
        default: {
            LogError()("opentxs::factory::")(__func__)(
                ": Unsupported address type")
                .Flush();

            return {};
        }
    }

    return std::make_unique<ReturnType>(
        api,
        config,
        mempool,
        network,
        header,
        filter,
        block,
        manager,
        policy,
        shutdown,
        id,
        std::move(address));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::implementation
{
const std::pmr::map<Command, Peer::CommandFunction> Peer::command_map_{
    {Command::addr, &Peer::process_addr},
    {Command::block, &Peer::process_block},
    {Command::blocktxn, &Peer::process_blocktxn},
    {Command::cfcheckpt, &Peer::process_cfcheckpt},
    {Command::cfheaders, &Peer::process_cfheaders},
    {Command::cfilter, &Peer::process_cfilter},
    {Command::cmpctblock, &Peer::process_cmpctblock},
    {Command::feefilter, &Peer::process_feefilter},
    {Command::filteradd, &Peer::process_filteradd},
    {Command::filterclear, &Peer::process_filterclear},
    {Command::filterload, &Peer::process_filterload},
    {Command::getaddr, &Peer::process_getaddr},
    {Command::getblocks, &Peer::process_getblocks},
    {Command::getblocktxn, &Peer::process_getblocktxn},
    {Command::getcfcheckpt, &Peer::process_getcfcheckpt},
    {Command::getcfheaders, &Peer::process_getcfheaders},
    {Command::getcfilters, &Peer::process_getcfilters},
    {Command::getdata, &Peer::process_getdata},
    {Command::getheaders, &Peer::process_getheaders},
    {Command::headers, &Peer::process_headers},
    {Command::inv, &Peer::process_inv},
    {Command::mempool, &Peer::process_mempool},
    {Command::merkleblock, &Peer::process_merkleblock},
    {Command::notfound, &Peer::process_notfound},
    {Command::ping, &Peer::process_ping},
    {Command::pong, &Peer::process_pong},
    {Command::reject, &Peer::process_reject},
    {Command::sendcmpct, &Peer::process_sendcmpct},
    {Command::sendheaders, &Peer::process_sendheaders},
    {Command::tx, &Peer::process_tx},
    {Command::verack, &Peer::process_verack},
    {Command::version, &Peer::process_version},
};

const std::string Peer::user_agent_{
    std::string{"/opentxs:"} + VersionString() + "/"};

Peer::Peer(
    const api::Session& api,
    const node::internal::Config& config,
    const node::internal::Mempool& mempool,
    const node::internal::Network& network,
    const node::HeaderOracle& header,
    const node::internal::FilterOracle& filter,
    const node::internal::BlockOracle& block,
    const node::internal::PeerManager& manager,
    const database::BlockStorage policy,
    const std::string& shutdown,
    const int id,
    std::unique_ptr<internal::Address> address,
    const bool relay,
    const std::pmr::set<p2p::Service>& localServices,
    const ProtocolVersion protocol) noexcept
    : p2p::implementation::Peer(
          api,
          config,
          mempool,
          network,
          filter,
          block,
          manager,
          id,
          shutdown,
          HeaderType::Size(),
          MessageType::MaxPayload(),
          std::move(address))
    , headers_(header)
    , protocol_((0 == protocol) ? default_protocol_version_ : protocol)
    , nonce_(nonce(api_))
    , local_services_(
          get_local_services(protocol_, chain_, policy, localServices))
    , relay_(relay)
    , get_headers_()
{
    init();
}

auto Peer::broadcast_block(zmq::Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (2 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid command").Flush();

        OT_FAIL;
    }

    const auto id = api_.Factory().Data(body.at(1));
    auto payload = [&] {
        using Inventory = blockchain::bitcoin::Inventory;
        auto output = std::pmr::vector<Inventory>{};
        output.emplace_back(Inventory::Type::MsgBlock, id);

        return output;
    }();
    auto pMsg = std::unique_ptr<Message>{
        factory::BitcoinP2PInv(api_, chain_, std::move(payload))};

    if (false == bool(pMsg)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct inv").Flush();

        return;
    }

    const auto& msg = *pMsg;
    LogTrace()("sending inv (block) message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    send(msg.Transmit());
}

auto Peer::broadcast_inv(
    std::pmr::vector<blockchain::bitcoin::Inventory>&& inv) noexcept -> void
{
    auto pMsg = std::unique_ptr<Message>{
        factory::BitcoinP2PInv(api_, chain_, std::move(inv))};

    if (false == bool(pMsg)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct inv").Flush();

        return;
    }

    const auto& msg = *pMsg;
    LogTrace()("sending inv message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    send(msg.Transmit());
}

auto Peer::broadcast_inv_transaction(ReadView txid) noexcept -> void
{
    using Inventory = blockchain::bitcoin::Inventory;
    using Type = Inventory::Type;
    const auto type = [&] {
        const auto& segwit = params::Data::Chains().at(chain_).segwit_;

        if (segwit) {

            return Type::MsgWitnessTx;
        } else {

            return Type::MsgTx;
        }
    }();
    auto inv = std::pmr::vector<Inventory>{};
    inv.emplace_back(type, api_.Factory().Data(txid));
    broadcast_inv(std::move(inv));
}

auto Peer::broadcast_transaction(zmq::Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (2 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid command").Flush();

        OT_FAIL;
    }

    auto pMsg = std::unique_ptr<Message>{
        factory::BitcoinP2PTx(api_, chain_, body.at(1).Bytes())};

    if (false == bool(pMsg)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct tx").Flush();

        return;
    }

    LogTrace()("sending tx message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    const auto& msg = *pMsg;
    send(msg.Transmit());
}

auto Peer::get_body_size(const zmq::Frame& header) const noexcept -> std::size_t
{
    OT_ASSERT(HeaderType::Size() == header.size());

    try {
        auto raw = HeaderType::BitcoinFormat{header};

        return raw.PayloadSize();
    } catch (...) {

        return 0;
    }
}

auto Peer::get_local_services(
    const ProtocolVersion version,
    const blockchain::Type network,
    const database::BlockStorage policy,
    const std::pmr::set<p2p::Service>& input) noexcept
    -> std::pmr::set<p2p::Service>
{
    auto output{input};

    switch (network) {
        case blockchain::Type::Bitcoin:
        case blockchain::Type::Bitcoin_testnet3:
        case blockchain::Type::Litecoin:
        case blockchain::Type::Litecoin_testnet4:
        case blockchain::Type::PKT:
        case blockchain::Type::PKT_testnet: {
            output.emplace(p2p::Service::Witness);
        } break;
        case blockchain::Type::BitcoinCash:
        case blockchain::Type::BitcoinCash_testnet3: {
            output.emplace(p2p::Service::BitcoinCash);
        } break;
        case blockchain::Type::Unknown:
        case blockchain::Type::Ethereum_frontier:
        case blockchain::Type::Ethereum_ropsten:
        case blockchain::Type::UnitTest:
        default: {
        }
    }

    switch (policy) {
        case database::BlockStorage::All: {
            output.emplace(p2p::Service::Network);
            output.emplace(p2p::Service::CompactFilters);
        } break;
        case database::BlockStorage::Cache: {
            output.emplace(p2p::Service::Limited);
            output.emplace(p2p::Service::CompactFilters);
        } break;
        default: {
        }
    }

    return output;
}

auto Peer::nonce(const api::Session& api) noexcept -> Nonce
{
    Nonce output{0};
    const auto random =
        api.Crypto().Util().RandomizeMemory(&output, sizeof(output));

    OT_ASSERT(random);

    return output;
}

auto Peer::ping() noexcept -> void
{
    std::unique_ptr<Message> pPing{
        factory::BitcoinP2PPing(api_, chain_, nonce_)};

    if (false == bool(pPing)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct ping").Flush();

        return;
    }

    LogTrace()("sending ping message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    const auto& ping = *pPing;
    send(ping.Transmit());
}

auto Peer::pong() noexcept -> void
{
    std::unique_ptr<Message> pPong{
        factory::BitcoinP2PPong(api_, chain_, nonce_)};

    if (false == bool(pPong)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct pong").Flush();

        return;
    }

    LogTrace()("sending pong message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    const auto& pong = *pPong;
    send(pong.Transmit());
}

auto Peer::process_addr(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Addr> pMessage{
        factory::BitcoinP2PAddr(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    download_peers_.Bump();
    using DB = blockchain::node::internal::PeerDatabase;
    auto peers = std::pmr::vector<DB::Address>{};

    for (const auto& address : message) {
        auto pAddress = address.clone_internal();

        OT_ASSERT(pAddress);

        pAddress->SetLastConnected({});
        peers.emplace_back(std::move(pAddress));
    }

    manager_.Database().Import(std::move(peers));
}

auto Peer::process_block(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    try {
        if (0 == payload.size()) {
            throw std::runtime_error("Invalid payload");
        }

        auto submit{true};
        auto block = api_.Factory().BitcoinBlock(chain_, payload.Bytes());

        if (!block) { throw std::runtime_error("Failed to instantiate block"); }

        if (false == block_.Validate(*block)) {
            throw std::runtime_error("Invalid block");
        }

        if (block_job_) {
            auto header = headers_.LoadHeader(block->Header().Hash());

            if (!header) { throw std::runtime_error("Failed to load header"); }

            submit = !block_job_.Download(header->Position(), std::move(block));

            if (block_job_.isDownloaded()) { reset_block_job(); }
        }

        if (submit) {
            using Task = node::internal::Network::Task;
            network_.Submit([&] {
                auto work = MakeWork(Task::SubmitBlock);
                work.AddFrame(payload);

                return work;
            }());
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Peer::process_blocktxn(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Blocktxn> pMessage{
        factory::BitcoinP2PBlocktxn(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_cfcheckpt(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Cfcheckpt> pMessage{
        factory::BitcoinP2PCfcheckpt(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_cfheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    auto& success = state_.verify_.second_action_;
    auto postcondition = ScopeGuard{[this, &success] {
        if (verifying() && (false == success)) {
            LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
                address_.Display())(" due to filter checkpoint failure.")
                .Flush();
            disconnect();
        }
    }};

    const auto pMessage = std::unique_ptr<message::internal::Cfheaders>{
        factory::BitcoinP2PCfheaders(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;

    if (verifying()) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Received checkpoint filter header message")
            .Flush();

        if (1 != pMessage->size()) {
            LogVerbose()(OT_PRETTY_CLASS())("Unexpected filter header count: ")(
                pMessage->size())
                .Flush();

            return;
        }

        const auto checkpointData = headers_.Internal().GetDefaultCheckpoint();
        const auto& [height, checkpointHash, parentHash, filterHash] =
            checkpointData;
        const auto receivedFilterHeader =
            blockchain::internal::FilterHashToHeader(
                api_, message.at(0).Bytes(), message.Previous().Bytes());

        if (filterHash != receivedFilterHeader) {
            LogVerbose()(OT_PRETTY_CLASS())("Unexpected filter header: ")(
                receivedFilterHeader->asHex())(". Expected: ")(
                filterHash->asHex())
                .Flush();

            return;
        }

        LogVerbose()("Filter checkpoint validated for ")(
            display_chain_)(" peer ")(address_.Display())
            .Flush();
        cfilter_probe_ = true;
        success = true;
        check_verify();
    } else if (cfheader_job_) {
        try {
            const auto hashCount = message.size();
            const auto headers = [&] {
                const auto stop = headers_.LoadHeader(message.Stop());

                if (false == bool(stop)) {
                    throw std::runtime_error("Stop block not found");
                }

                if (0 > stop->Height()) {
                    throw std::runtime_error("Stop block disconnected");
                }

                if (hashCount > static_cast<std::size_t>(stop->Height())) {
                    throw std::runtime_error(
                        "Too many filter headers returned");
                }

                const auto startHeight =
                    stop->Height() - static_cast<block::Height>(hashCount) + 1;
                const auto start =
                    headers_.LoadHeader(headers_.BestHash(startHeight));

                if (false == bool(start)) {
                    throw std::runtime_error("Start block not found");
                }

                auto output =
                    headers_.Ancestors(start->Position(), stop->Position());

                while (output.size() > hashCount) {
                    output.erase(output.begin());
                }

                return output;
            }();

            if (headers.size() != hashCount) {
                throw std::runtime_error(
                    "Failed to load all block positions for cfheader message");
            }

            auto block = headers.begin();
            auto cfheader = message.begin();
            const auto type = message.Type();

            for (; cfheader != message.end(); ++block, ++cfheader) {
                cfheader_job_.Download(*block, std::move(*cfheader), type);
            }

            if (cfheader_job_.isDownloaded()) { reset_cfheader_job(); }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            reset_cfheader_job();
        }
    }
}

auto Peer::process_cfilter(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    try {
        const auto pMessage = std::unique_ptr<message::internal::Cfilter>{
            factory::BitcoinP2PCfilter(
                api_,
                std::move(header),
                protocol_.load(),
                payload.data(),
                payload.size())};

        if (false == bool(pMessage)) {
            throw std::runtime_error("Failed to decode cfilter payload");
        }

        if (cfilter_job_) {
            const auto& message = *pMessage;
            const auto block = headers_.LoadHeader(message.Hash());

            if (false == bool(block)) {
                throw std::runtime_error("Failed to load block header");
            }

            const auto& blockHash = message.Hash();
            auto gcs = factory::GCS(
                api_,
                message.Bits(),
                message.FPRate(),
                blockchain::internal::BlockHashToFilterKey(blockHash.Bytes()),
                message.ElementCount(),
                message.Filter());

            if (false == bool(gcs)) {
                throw std::runtime_error("Failed to instantiate gcs");
            }

            cfilter_job_.Download(
                block->Position(), std::move(gcs), message.Type());

            if (cfilter_job_.isDownloaded()) { reset_cfilter_job(); }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Peer::process_cmpctblock(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Cmpctblock> pMessage{
        factory::BitcoinP2PCmpctblock(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_feefilter(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Feefilter> pMessage{
        factory::BitcoinP2PFeefilter(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_filteradd(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Filteradd> pMessage{
        factory::BitcoinP2PFilteradd(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_filterclear(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Filterclear> pMessage{
        factory::BitcoinP2PFilterclear(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_filterload(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Filterload> pMessage{
        factory::BitcoinP2PFilterload(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_getaddr(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getaddr> pMessage{
        factory::BitcoinP2PGetaddr(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_getblocks(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Getblocks> pMessage{
        factory::BitcoinP2PGetblocks(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_getblocktxn(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Getblocktxn> pMessage{
        factory::BitcoinP2PGetblocktxn(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_getcfcheckpt(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getcfcheckpt> pMessage{
        factory::BitcoinP2PGetcfcheckpt(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_getcfheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const auto pIn = std::unique_ptr<message::internal::Getcfheaders>{
        factory::BitcoinP2PGetcfheaders(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pIn)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& in = *pIn;

    if (false == bool(headers_.LoadHeader(in.Stop()))) { return; }

    const auto fromGenesis = (0 == in.Start());
    const auto blocks = headers_.BestHashes(
        fromGenesis ? 0 : in.Start() - 1, in.Stop(), fromGenesis ? 2000 : 2001);

    if (0 == blocks.size()) { return; }

    const auto& fOracle = network_.FilterOracleInternal();
    const auto filterType = in.Type();
    const auto previousHeader =
        fOracle.LoadFilterHeader(filterType, *blocks.cbegin());

    if (previousHeader->empty()) { return; }

    auto filterHashes =
        std::pmr::vector<node::internal::FilterOracle::Header>{};
    const auto start = std::size_t{fromGenesis ? 0u : 1u};
    static const auto blank = std::array<char, 32>{};
    const auto previous = fromGenesis ? ReadView{blank.data(), blank.size()}
                                      : previousHeader->Bytes();

    for (auto i{start}; i < blocks.size(); ++i) {
        const auto& blockHash = blocks.at(i);
        const auto pFilter = fOracle.LoadFilter(filterType, blockHash);

        if (false == bool(pFilter)) { break; }

        const auto& filter = *pFilter;
        filterHashes.emplace_back(filter.Hash());
    }

    if (0 == filterHashes.size()) { return; }

    auto pOut = std::unique_ptr<
        message::internal::Cfheaders>{factory::BitcoinP2PCfheaders(
        api_, chain_, in.Type(), in.Stop(), previous, std::move(filterHashes))};

    if (false == bool(pOut)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct reply").Flush();

        return;
    }

    LogTrace()("sending cfheaders message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    const auto& message = *pOut;
    send(message.Transmit());
}

auto Peer::process_getcfilters(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getcfilters> pMessage{
        factory::BitcoinP2PGetcfilters(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    const auto& stopHash = message.Stop();
    const auto pStopHeader = headers_.LoadHeader(stopHash);

    if (!pStopHeader) {
        LogError()(OT_PRETTY_CLASS())(
            "Skipping request with unknown stop header")
            .Flush();

        return;
    }

    const auto& stopHeader = *pStopHeader;
    const auto startHeight{message.Start()};
    const auto stopHeight{stopHeader.Height()};

    if (startHeight > stopHeight) {
        LogError()(OT_PRETTY_CLASS())(
            "Skipping request with malformed start height (")(
            startHeight)(") vs stop (")(stopHeight)(")")
            .Flush();

        return;
    }

    if (0 > startHeight) {
        LogError()(OT_PRETTY_CLASS())(
            "Skipping request with negative start height (")(startHeight)(")")
            .Flush();

        return;
    }

    constexpr auto limit = std::size_t{1000};
    const auto count =
        static_cast<std::size_t>((stopHeight - startHeight) + 1u);

    if (count > limit) {
        LogError()(OT_PRETTY_CLASS())(
            "Skipping request with excessive filter requests (")(
            count)(") vs allowed (")(limit)(")")
            .Flush();

        return;
    }

    auto data = std::pmr::vector<std::unique_ptr<const GCS>>{};
    data.reserve(count);
    const auto& filters = network_.FilterOracle();
    const auto type = message.Type();
    const auto hashes = headers_.BestHashes(startHeight, stopHash);

    for (const auto& hash : hashes) {
        const auto& pGCS = data.emplace_back(filters.LoadFilter(type, hash));

        if (!pGCS) { break; }
    }

    if (data.size() != count) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to load all filters, requested (")(count)("), loaded (")(
            data.size())(")")
            .Flush();

        return;
    }

    OT_ASSERT(data.size() == hashes.size());

    auto h{hashes.begin()};

    for (auto g{data.begin()}; g != data.end(); ++g, ++h) {
        auto pOut = std::unique_ptr<message::internal::Cfilter>{
            factory::BitcoinP2PCfilter(api_, chain_, type, *h, *(*g))};

        if (false == bool(pOut)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct reply").Flush();

            return;
        }

        LogTrace()("sending cfilter message to ")(display_chain_)(" peer ")(
            address_.Display())
            .Flush();
        const auto& message = *pOut;
        send(message.Transmit());
    }
}

auto Peer::process_getdata(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Getdata> pMessage{
        factory::BitcoinP2PGetdata(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    using Type = blockchain::bitcoin::Inventory::Type;
    auto notFound = std::pmr::vector<blockchain::bitcoin::Inventory>{};

    for (const auto& inv : message) {
        switch (inv.type_) {
            case Type::MsgTx: {
                auto tx = mempool_.Query(inv.hash_->Bytes());

                if (tx) {
                    known_transactions_.emplace(inv.hash_->Bytes());
                    const auto bytes = [&] {
                        auto out = Space{};
                        tx->Internal().Serialize(writer(out));

                        return out;
                    }();
                    const auto pMsg = std::unique_ptr<Message>{
                        factory::BitcoinP2PTx(api_, chain_, reader(bytes))};

                    OT_ASSERT(pMsg);

                    LogTrace()("sending tx message to ")(
                        display_chain_)(" peer ")(address_.Display())
                        .Flush();

                    const auto& msg = *pMsg;
                    send(msg.Transmit());
                } else {
                    notFound.emplace_back(inv);
                }
            } break;
            case Type::MsgBlock: {
                const auto& oracle = network_.BlockOracle();
                auto future = oracle.LoadBitcoin(inv.hash_);
                const auto have = std::future_status::ready ==
                                  future.wait_for(std::chrono::milliseconds{0});

                if (have) {
                    const auto pBlock = future.get();

                    OT_ASSERT(pBlock);

                    const auto& block = *pBlock;
                    const auto serialized = [&] {
                        auto output = api_.Factory().Data();
                        block.Serialize(output->WriteInto());

                        return output;
                    }();
                    const auto pMsg = std::unique_ptr<Message>{
                        factory::BitcoinP2PBlock(api_, chain_, serialized)};

                    OT_ASSERT(pMsg);

                    LogTrace()("sending block message to ")(
                        display_chain_)(" peer ")(address_.Display())
                        .Flush();

                    const auto& msg = *pMsg;
                    send(msg.Transmit());
                } else {
                    notFound.emplace_back(inv);
                }
            } break;
            case Type::None:
            case Type::MsgFilteredBlock:
            case Type::MsgCmpctBlock:
            case Type::MsgWitnessTx:
            case Type::MsgWitnessBlock:
            case Type::MsgFilteredWitnessBlock:
            default: {
                notFound.emplace_back(inv);
            }
        }
    }

    if (0 < notFound.size()) {
        const auto pMsg = std::unique_ptr<Message>{
            factory::BitcoinP2PNotfound(api_, chain_, std::move(notFound))};

        OT_ASSERT(pMsg);

        LogTrace()("sending notfound message to ")(display_chain_)(" peer ")(
            address_.Display())
            .Flush();

        const auto& msg = *pMsg;
        send(msg.Transmit());
    }
}

auto Peer::process_getheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const auto pIn = std::unique_ptr<message::internal::Getheaders>{
        factory::BitcoinP2PGetheaders(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pIn)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& in = *pIn;
    auto previous = node::HeaderOracle::Hashes{};
    std::copy(in.begin(), in.end(), std::back_inserter(previous));
    const auto hashes = headers_.BestHashes(previous, in.StopHash(), 2000);
    auto headers = std::pmr::vector<std::unique_ptr<block::bitcoin::Header>>{};
    std::transform(
        hashes.begin(),
        hashes.end(),
        std::back_inserter(headers),
        [&](const auto& hash) -> auto {
            return headers_.Internal().LoadBitcoinHeader(hash);
        });
    auto pOut = std::unique_ptr<message::internal::Headers>{
        factory::BitcoinP2PHeaders(api_, chain_, std::move(headers))};

    if (false == bool(pOut)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct reply").Flush();

        return;
    }

    LogTrace()("sending headers message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();

    const auto& message = *pOut;
    send(message.Transmit());
}

auto Peer::process_headers(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    auto& success = state_.verify_.first_action_;
    auto postcondition = ScopeGuard{[this, &success] {
        if (verifying() && (false == success)) {
            LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
                address_.Display())(" due to block checkpoint failure.")
                .Flush();
            disconnect();
        }
    }};

    const auto pMessage =
        std::unique_ptr<message::internal::Headers>{factory::BitcoinP2PHeaders(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogVerbose()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;

    if (verifying()) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Received checkpoint block header message")
            .Flush();

        if (1 != pMessage->size()) {
            LogVerbose()(OT_PRETTY_CLASS())("Unexpected header count: ")(
                pMessage->size())
                .Flush();

            return;
        }

        const auto checkpointData = headers_.Internal().GetDefaultCheckpoint();
        const auto& [height, checkpointHash, parentHash, filterHash] =
            checkpointData;
        const auto& receivedBlockHash = message.at(0).Hash();

        if (checkpointHash != receivedBlockHash) {
            LogVerbose()(OT_PRETTY_CLASS())("Unexpected block header hash: ")(
                receivedBlockHash.asHex())(". Expected: ")(
                checkpointHash->asHex())
                .Flush();

            return;
        }

        LogVerbose()("Block checkpoint validated for ")(
            display_chain_)(" peer ")(address_.Display())
            .Flush();
        header_probe_ = true;
        success = true;
        check_verify();
    } else {
        get_headers_.Finish();
        auto future = network_.Track([&] {
            using Task = node::internal::Network::Task;
            auto work = MakeWork(Task::SubmitBlockHeader);

            for (const auto& header : message) {
                header.Serialize(work.AppendBytes(), false);
            }

            return work;
        }());
        using Status = std::future_status;
        constexpr auto limit = std::chrono::seconds{10};

        if (Status::ready == future.wait_for(limit)) {
            if (false == network_.IsSynchronized()) { request_headers(); }
        }
    }
}

auto Peer::process_inv(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    if (false == running_.load()) { return; }
    if (State::Run != state_.value_.load()) { return; }

    const std::unique_ptr<message::internal::Inv> pMessage{
        factory::BitcoinP2PInv(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;
    using Inventory = blockchain::bitcoin::Inventory;
    using Type = Inventory::Type;
    auto txReceived = std::pmr::vector<Inventory>{};
    auto txToDownload = std::pmr::vector<Inventory>{};

    for (const auto& inv : message) {
        const auto& hash = inv.hash_.get();
        LogVerbose()("Received ")(display_chain_)(" ")(inv.DisplayType())(
            " hash ")(hash.asHex())
            .Flush();

        switch (inv.type_) {
            case Type::MsgBlock:
            case Type::MsgWitnessBlock: {
                request_headers(hash);
            } break;
            case Type::MsgTx:
            case Type::MsgWitnessTx: {
                known_transactions_.emplace(hash.Bytes());
                txReceived.emplace_back(inv);
            } break;
            default: {
            }
        }
    }

    if (0 < txReceived.size()) {
        const auto hashes = [&] {
            auto out = std::pmr::vector<ReadView>{};
            std::transform(
                txReceived.begin(),
                txReceived.end(),
                std::back_inserter(out),
                [&](const auto& in) { return in.hash_->Bytes(); });

            return out;
        }();
        const auto result = mempool_.Submit(hashes);

        OT_ASSERT(txReceived.size() == result.size());

        for (auto i{0u}; i < result.size(); ++i) {
            const auto& download = result.at(i);

            if (download) { txToDownload.emplace_back(txReceived.at(i)); }
        }
    }

    if (0 < txToDownload.size()) {
        request_transactions(std::move(txToDownload));
    }
}

auto Peer::process_mempool(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Mempool> pMessage{
        factory::BitcoinP2PMempool(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    reconcile_mempool();
}

auto Peer::process_merkleblock(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Merkleblock> pMessage{
        factory::BitcoinP2PMerkleblock(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_message(zmq::Message&& message) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = message.Body();

    if (3 > body.size()) {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" due to invalid message.")
            .Flush();
        disconnect();

        return;
    }

    const auto& headerBytes = body.at(1);
    const auto& payloadBytes = body.at(2);
    auto pHeader = std::unique_ptr<HeaderType>{
        factory::BitcoinP2PHeader(api_, headerBytes)};

    if (false == bool(pHeader)) {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" due to invalid message header.")
            .Flush();
        disconnect();

        return;
    }

    auto& header = *pHeader;

    if (header.Network() != chain_) {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" due to invalid network.")
            .Flush();
        disconnect();

        return;
    }

    const auto command = header.Command();

    if (false == message::VerifyChecksum(api_, header, payloadBytes)) {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" due to invalid message checksum.")
            .Flush();
        disconnect();

        return;
    }

    LogTrace()(OT_PRETTY_CLASS())("Received ")(display_chain_)(" ")(
        CommandName(command))(" command from ")(address_.Display())
        .Flush();

    try {
        const auto& p = command_map_.at(command);
        (this->*p)(std::move(pHeader), payloadBytes);
    } catch (...) {
        auto raw = Data::Factory(headerBytes);
        auto unknown = Data::Factory();
        raw->Extract(12, unknown, 4);
        LogError()(OT_PRETTY_CLASS())("No handler for command ")(unknown->str())
            .Flush();

        return;
    }
}

auto Peer::process_notfound(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Notfound> pMessage{
        factory::BitcoinP2PNotfound(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_ping(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Ping> pMessage{
        factory::BitcoinP2PPing(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;

    if (message.Nonce() == nonce_) {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" which is apparently us.")
            .Flush();
        disconnect();
    }

    pong();
}

auto Peer::process_pong(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Pong> pMessage{
        factory::BitcoinP2PPong(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }
}

auto Peer::process_reject(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Reject> pMessage{factory::BitcoinP2PReject(
        api_,
        std::move(header),
        protocol_.load(),
        payload.data(),
        payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_sendcmpct(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::Sendcmpct> pMessage{
        factory::BitcoinP2PSendcmpct(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_sendheaders(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Sendheaders> pMessage{
        factory::BitcoinP2PSendheaders(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    // TODO
}

auto Peer::process_tx(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const auto pMessage = factory::BitcoinP2PTx(
        api_,
        std::move(header),
        protocol_.load(),
        payload.data(),
        payload.size());

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& message = *pMessage;

    if (auto tx = message.Transaction(); tx) {
        known_transactions_.emplace(tx->ID().Bytes());
        mempool_.Submit(message.Transaction());
    }
}

auto Peer::process_verack(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Verack> pMessage{
        factory::BitcoinP2PVerack(api_, std::move(header))};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    state_.handshake_.first_action_ = true;
    check_handshake();
}

auto Peer::process_version(
    std::unique_ptr<HeaderType> header,
    const zmq::Frame& payload) -> void
{
    const std::unique_ptr<message::internal::Version> pVersion{
        factory::BitcoinP2PVersion(
            api_,
            std::move(header),
            protocol_.load(),
            payload.data(),
            payload.size())};

    if (false == bool(pVersion)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message payload")
            .Flush();

        return;
    }

    const auto& version = *pVersion;
    network_.UpdateHeight(version.Height());
    protocol_.store(std::min(protocol_.load(), version.ProtocolVersion()));
    const auto services = version.RemoteServices();
    update_address_services(services);
    std::unique_ptr<Message> pVerack{factory::BitcoinP2PVerack(api_, chain_)};

    if (false == bool(pVerack)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct verack").Flush();

        return;
    }

    const auto& verack = *pVerack;
    LogTrace()("sending verack message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    send(verack.Transmit());
    state_.handshake_.second_action_ = true;

    if (address_.Incoming()) { start_handshake(); }

    check_handshake();
}

auto Peer::reconcile_mempool() noexcept -> void
{
    const auto local = mempool_.Dump();
    const auto remote = [&] {
        auto out = std::pmr::set<std::string>{};
        std::copy(
            known_transactions_.begin(),
            known_transactions_.end(),
            std::inserter(out, out.end()));

        return out;
    }();
    const auto missing = [&] {
        auto out = std::pmr::vector<std::string>{};
        out.reserve(local.size());
        std::set_difference(
            local.begin(),
            local.end(),
            remote.begin(),
            remote.end(),
            std::back_inserter(out));

        return out;
    }();
    using Inventory = blockchain::bitcoin::Inventory;
    using Type = Inventory::Type;
    const auto type = [&] {
        const auto& segwit = params::Data::Chains().at(chain_).segwit_;

        if (segwit) {

            return Type::MsgWitnessTx;
        } else {

            return Type::MsgTx;
        }
    }();
    auto inv = std::pmr::vector<Inventory>{};

    for (const auto& hash : missing) {
        inv.emplace_back(type, api_.Factory().Data(hash));
    }

    broadcast_inv(std::move(inv));
}

auto Peer::request_addresses() noexcept -> void
{
    auto pMessage =
        std::unique_ptr<Message>{factory::BitcoinP2PGetaddr(api_, chain_)};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct getaddr").Flush();

        return;
    }

    LogTrace()("sending getaddr message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    const auto& message = *pMessage;
    send(message.Transmit());
}

auto Peer::request_block(zmq::Message&& in) noexcept -> void
{
    const auto body = in.Body();

    if (const auto count{body.size()}; 2 > count) {
        LogError()(OT_PRETTY_CLASS())("Invalid command").Flush();

        OT_FAIL;
    } else {
        LogTrace()(OT_PRETTY_CLASS())(count - 1)(" blocks to request").Flush();
    }

    using Inventory = blockchain::bitcoin::Inventory;
    using Type = Inventory::Type;
    using BlockList = std::pmr::vector<Inventory>;
    auto blocks = std::pmr::vector<BlockList>{};
    blocks.emplace_back();
    static constexpr auto limit = std::size_t{50000};

    for (auto i = std::size_t{1}; i < body.size(); ++i) {
        auto& list = blocks.back();
        list.emplace_back(Type::MsgBlock, api_.Factory().Data(body.at(1)));

        if (limit <= list.size()) { blocks.emplace_back(); }
    }

    for (auto& list : blocks) {
        auto pMessage = std::unique_ptr<Message>{
            factory::BitcoinP2PGetdata(api_, chain_, std::move(list))};

        if (false == bool(pMessage)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct getdata")
                .Flush();

            return;
        }

        LogTrace()("sending getdata(block) message to ")(
            display_chain_)(" peer ")(address_.Display())
            .Flush();
        const auto& message = *pMessage;
        send(message.Transmit());
    }
}

auto Peer::request_blocks() noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto& job = block_job_;
    const auto& data = job.data_;

    try {
        using Inventory = blockchain::bitcoin::Inventory;
        using Type = Inventory::Type;
        auto blocks = std::pmr::vector<Inventory>{};

        for (const auto& task : data) {
            blocks.emplace_back(Type::MsgBlock, task->position_.second);
        }

        if (0 == blocks.size()) { return; }

        auto pMessage = std::unique_ptr<Message>{
            factory::BitcoinP2PGetdata(api_, chain_, std::move(blocks))};

        if (false == bool(pMessage)) {
            throw std::runtime_error("Failed to construct getdata");
        }

        LogTrace()("sending getdata(block) message to ")(
            display_chain_)(" peer ")(address_.Display())
            .Flush();
        const auto& message = *pMessage;
        send(message.Transmit());
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Peer::request_cfheaders() noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto& job = cfheader_job_;
    const auto& data = job.data_;

    OT_ASSERT(0 < data.size());

    try {
        auto pMessage =
            std::unique_ptr<Message>{factory::BitcoinP2PGetcfheaders(
                api_,
                chain_,
                job.extra_,
                data.front()->position_.first,
                data.back()->position_.second)};

        if (false == bool(pMessage)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct getcfheaders")
                .Flush();

            return;
        }

        LogTrace()("sending getcfheaders message to ")(
            display_chain_)(" peer ")(address_.Display())
            .Flush();
        const auto& message = *pMessage;
        send(message.Transmit());
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Invalid parameters").Flush();
    }
}

auto Peer::request_cfilter() noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto& job = cfilter_job_;
    const auto& data = job.data_;

    OT_ASSERT(0 < data.size());

    try {
        auto pMessage = std::unique_ptr<Message>{factory::BitcoinP2PGetcfilters(
            api_,
            chain_,
            job.extra_,
            data.front()->position_.first,
            data.back()->position_.second)};

        if (false == bool(pMessage)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct getcfilters")
                .Flush();

            return;
        }

        LogTrace()("sending getcfilters message to ")(display_chain_)(" peer ")(
            address_.Display())
            .Flush();
        const auto& message = *pMessage;
        send(message.Transmit());
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Invalid parameters").Flush();
    }
}

auto Peer::request_checkpoint_block_header() noexcept -> void
{
    auto success = false;
    auto postcondition = ScopeGuard{[this, &success] {
        if (success) {
            LogVerbose()("Requested checkpoint block header from ")(
                display_chain_)(" peer ")(address_.Display())(".")
                .Flush();
        } else {
            LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
                address_.Display())(" due to block checkpoint request failure.")
                .Flush();
            disconnect();
        }
    }};

    try {
        auto checkpointData = headers_.Internal().GetDefaultCheckpoint();
        auto [height, checkpointBlockHash, parentBlockHash, filterHash] =
            checkpointData;
        auto pMessage = std::unique_ptr<Message>{factory::BitcoinP2PGetheaders(
            api_,
            chain_,
            protocol_.load(),
            {std::move(parentBlockHash)},
            std::move(checkpointBlockHash))};

        if (false == bool(pMessage)) {
            LogVerbose()(OT_PRETTY_CLASS())("Failed to construct getheaders")
                .Flush();

            return;
        }

        LogTrace()("sending getheaders message to ")(display_chain_)(" peer ")(
            address_.Display())
            .Flush();
        const auto& message = *pMessage;
        send(message.Transmit());
        success = true;
    } catch (...) {
    }
}

auto Peer::request_checkpoint_filter_header() noexcept -> void
{
    auto success = false;
    auto postcondition = ScopeGuard{[this, &success] {
        if (success) {
            LogVerbose()("Requested checkpoint filter header from ")(
                display_chain_)(" peer ")(address_.Display())(".")
                .Flush();
        } else {
            LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
                address_.Display())(
                " due to filter checkpoint request failure.")
                .Flush();
            disconnect();
        }
    }};

    try {
        auto checkpointData = headers_.Internal().GetDefaultCheckpoint();
        auto [height, checkpointBlockHash, parentBlockHash, filterHash] =
            checkpointData;
        auto pMessage =
            std::unique_ptr<Message>{factory::BitcoinP2PGetcfheaders(
                api_,
                chain_,
                network_.FilterOracleInternal().DefaultType(),
                height,
                checkpointBlockHash)};

        if (false == bool(pMessage)) {
            LogVerbose()(OT_PRETTY_CLASS())("Failed to construct getcfheaders")
                .Flush();

            return;
        }

        LogTrace()("sending getcfheaders message to ")(
            display_chain_)(" peer ")(address_.Display())
            .Flush();
        const auto& message = *pMessage;
        send(message.Transmit());
        success = true;
    } catch (...) {
        LogVerbose()(OT_PRETTY_CLASS())("Invalid parameters").Flush();
    }
}

auto Peer::request_headers() noexcept -> void
{
    request_headers(api_.Factory().Data());
}

auto Peer::request_headers(const block::Hash& hash) noexcept -> void
{
    if (get_headers_.Running()) { return; }

    auto pMessage = std::unique_ptr<Message>{factory::BitcoinP2PGetheaders(
        api_, chain_, protocol_.load(), headers_.RecentHashes(), hash)};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct getheaders").Flush();

        return;
    }

    LogTrace()("sending getheaders message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    const auto& message = *pMessage;
    send(message.Transmit());
    get_headers_.Start();
}

auto Peer::request_mempool() noexcept -> void
{
    auto pMessage =
        std::unique_ptr<Message>{factory::BitcoinP2PMempool(api_, chain_)};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct mempool").Flush();

        return;
    }

    LogTrace()("sending mempool message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    const auto& message = *pMessage;
    send(message.Transmit());
}

auto Peer::request_transactions(
    std::pmr::vector<blockchain::bitcoin::Inventory>&& inv) noexcept -> void
{
    auto pMessage = std::unique_ptr<Message>{
        factory::BitcoinP2PGetdata(api_, chain_, std::move(inv))};

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct getdata").Flush();

        return;
    }

    LogTrace()("sending getdata(tx) message to ")(display_chain_)(" peer ")(
        address_.Display())
        .Flush();
    const auto& message = *pMessage;
    send(message.Transmit());
}

auto Peer::start_handshake() noexcept -> void
{
    try {
        const auto status = Connected().wait_for(std::chrono::seconds(5));

        if (std::future_status::ready != status) {
            LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
                address_.Display())(" due to handshake timeout.")
                .Flush();
            disconnect();

            return;
        }
    } catch (const std::exception& e) {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" due to handshake error: ")(e.what())
            .Flush();
        disconnect();

        return;
    }

    try {
        const auto& address = connection();
        const auto local = address.endpoint_data();
        std::unique_ptr<Message> pVersion{factory::BitcoinP2PVersion(
            api_,
            chain_,
            address.style(),
            protocol_.load(),
            local_services_,
            local.first,
            local.second,
            address_.Services(),
            address.address(),
            address.port(),
            nonce_,
            user_agent_,
            network_.GetHeight(),
            relay_)};

        OT_ASSERT(pVersion);

        LogTrace()("sending version message to ")(display_chain_)(" peer ")(
            address_.Display())
            .Flush();
        const auto& version = *pVersion;
        send(version.Transmit());
    } catch (const std::exception& e) {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" due to handshake error: ")(e.what())
            .Flush();
        disconnect();
    }
}

Peer::~Peer() { Shutdown(); }
}  // namespace opentxs::blockchain::p2p::bitcoin::implementation
