// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/manager/Shared.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "blockchain/node/Mempool.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Factory.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/network/blockchain/Address.hpp"
#include "internal/network/blockchain/OTDHT.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/otdht/PushTransaction.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/BlockchainProfile.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::manager
{
Shared::Shared(
    const api::session::Client& api,
    const Type type,
    const node::internal::Config& config,
    std::string_view seednode,
    node::Endpoints endpoints) noexcept
    : api_(api)
    , chain_(type)
    , config_(config)
    , endpoints_(std::move(endpoints))
    , filter_type_([&] {
        using enum BlockchainProfile;

        switch (config_.profile_) {
            case mobile:
            case desktop: {

                return cfilter::Type::ES;
            }
            case desktop_native:
            case server: {

                return blockchain::internal::DefaultFilter(chain_);
            }
            default: {
                LogAbort()().Abort();
            }
        }
    }())
    , command_line_peers_(seednode)
    , shutdown_sender_(
          api.Network().Asio(),
          api.Network().ZeroMQ(),
          endpoints_.shutdown_publish_,
          CString{print(chain_)}
              .append(" on api instance ")
              .append(std::to_string(api_.Instance())))
    , database_(factory::BlockchainDatabase(
          api_,
          endpoints_,
          api_.Network().Blockchain().Internal().Database(),
          chain_,
          filter_type_))
    , mempool_(std::make_unique<node::Mempool>(
          api_,
          api_.Crypto().Blockchain(),
          chain_,
          *database_))
    , header_(factory::HeaderOracle(api_, chain_, endpoints_, *database_))
    , block_()
    , filter_(factory::BlockchainFilterOracle(
          api_,
          header_,
          endpoints_,
          config_,
          *database_,
          chain_,
          filter_type_))
    , wallet_()
    , shutdown_()
    , data_(api, endpoints_)
{
    assert_false(nullptr == database_);
    assert_false(nullptr == mempool_);
    assert_false(nullptr == filter_);
}

auto Shared::AddBlock(const block::Block& block) const noexcept -> bool
{
    if (false == block.IsValid()) {
        LogError()()("invalid ")(print(chain_))(" block").Flush();

        return false;
    }

    const auto& id = block.ID();

    if (false == block_.SubmitBlock(block, {})) {  // TODO monotonic allocator
        LogError()()("failed to save ")(print(chain_))(" block ")
            .asHex(id)
            .Flush();

        return false;
    }

    // TODO monotonic allocator
    if (false == filter_->Internal().ProcessBlock(block, {})) {
        LogError()()("failed to index ")(print(chain_))(" block").Flush();

        return false;
    }

    if (false == header_.Internal().AddHeader(block.Header())) {
        LogError()()("failed to process ")(print(chain_))(" header").Flush();

        return false;
    }

    return true;
}

auto Shared::AddPeer(const network::blockchain::Address& address) const noexcept
    -> bool
{
    try {
        const auto proto = [&] {
            auto out = proto::BlockchainPeerAddress{};

            if (false == address.Internal().Serialize(out)) {
                throw std::runtime_error{
                    "failed to serialize address to protobuf"};
            }

            return out;
        }();
        using enum PeerManagerJobs;
        auto work = MakeWork(addpeer);

        if (false == proto::write(proto, work.AppendBytes())) {
            throw std::runtime_error{"failed to serialize protobuf to bytes"};
        }

        return data_.lock()->to_peer_manager_.SendDeferred(std::move(work));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto Shared::BlockOracle() const noexcept -> const node::BlockOracle&
{
    return block_;
}

auto Shared::BroadcastTransaction(
    const block::Transaction& tx,
    const bool pushtx) const noexcept -> bool
{
    mempool_->Submit(tx);
    auto handle = data_.lock();
    auto& data = *handle;

    if (pushtx) {
        data.to_dht_.SendDeferred([&] {
            auto out = network::zeromq::Message{};
            const auto command =
                factory::BlockchainSyncPushTransaction(chain_, tx);
            command.Serialize(out);

            return out;
        }());
    }

    // TODO upgrade mempool logic so this becomes unnecessary

    using enum PeerManagerJobs;
    auto message = MakeWork(broadcasttx);

    if (false == tx.Internal()
                     .asBitcoin()
                     .Serialize(message.AppendBytes())
                     .has_value()) {

        return false;
    }

    return data.to_peer_manager_.SendDeferred(std::move(message));
}

auto Shared::Chain() const noexcept -> Type { return chain_; }

auto Shared::DB() const noexcept -> database::Database& { return *database_; }

auto Shared::Endpoints() const noexcept -> const node::Endpoints&
{
    return endpoints_;
}

auto Shared::FeeRate() const noexcept -> Amount
{
    // TODO in full node mode, calculate the fee network from the mempool and
    // recent blocks
    // TODO on networks that support it, query the fee rate from network peers
    const auto http = wallet_.Internal().FeeEstimate();
    const auto fallback = params::get(chain_).FallbackTxFeeRate();
    const auto chain = print(chain_);
    LogConsole()(chain)(" defined minimum fee rate is: ")(fallback).Flush();

    if (http.has_value()) {
        LogConsole()(chain)(" transaction fee rate via https oracle is: ")(
            http.value())
            .Flush();
    } else {
        LogConsole()(chain)(
            " transaction fee estimates via https oracle not available")
            .Flush();
    }

    auto out = std::max<Amount>(fallback, http.value_or(0));
    LogConsole()("Using ")(out)(" for current ")(chain)(" fee rate").Flush();

    return out;
}

auto Shared::FilterOracle() const noexcept -> const node::FilterOracle&
{
    return *filter_;
}

auto Shared::FilterOracle() noexcept -> node::FilterOracle& { return *filter_; }

auto Shared::GetBalance() const noexcept -> Balance
{
    return database_->GetBalance();
}

auto Shared::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    return database_->GetBalance(owner);
}

auto Shared::GetConfig() const noexcept -> const internal::Config&
{
    return config_;
}

auto Shared::GetShared() const noexcept -> std::shared_ptr<const node::Manager>
{
    return data_.lock()->self_.lock();
}

auto Shared::GetTransactions() const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return database_->GetTransactions();
}

auto Shared::GetTransactions(const identifier::Nym& account) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return database_->GetTransactions(account);
}

auto Shared::HeaderOracle() const noexcept -> const node::HeaderOracle&
{
    return header_;
}

auto Shared::HeaderOracle() noexcept -> node::HeaderOracle& { return header_; }

auto Shared::Init(std::shared_ptr<node::Manager> self) noexcept -> void
{
    assert_false(nullptr == self);

    header_.Internal().Init();
    auto api = api_.Internal().asClient().SharedClient();
    opentxs::network::blockchain::OTDHT{api, self}.Init();
    block_.Start(api, self);
    filter_->Internal().Init(api, self);
    header_.Start(api, self);
    factory::BlockchainPeerManager(api, self, *database_, command_line_peers_);
    wallet_.Internal().Init(api, self);
}

auto Shared::Listen(const network::blockchain::Address& address) const noexcept
    -> bool
{
    try {
        const auto proto = [&] {
            auto out = proto::BlockchainPeerAddress{};

            if (false == address.Internal().Serialize(out)) {
                throw std::runtime_error{
                    "failed to serialize address to protobuf"};
            }

            return out;
        }();
        using enum PeerManagerJobs;
        auto work = MakeWork(addlistener);

        if (false == proto::write(proto, work.AppendBytes())) {
            throw std::runtime_error{"failed to serialize protobuf to bytes"};
        }

        return data_.lock()->to_peer_manager_.SendDeferred(std::move(work));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto Shared::Mempool() const noexcept -> const internal::Mempool&
{
    return *mempool_;
}

auto Shared::Mempool() noexcept -> internal::Mempool& { return *mempool_; }

auto Shared::Profile() const noexcept -> BlockchainProfile
{
    return config_.profile_;
}

auto Shared::Shutdown() noexcept -> void
{
    auto shutdown = [this] {
        data_.lock()->self_.reset();
        shutdown_sender_.Activate();
        shutdown_sender_.Close();
    };
    std::call_once(shutdown_, shutdown);
}

auto Shared::ShuttingDown() const noexcept -> bool
{
    return shutdown_sender_.Activated();
}

auto Shared::StartWallet() noexcept -> void
{
    if (false == config_.disable_wallet_) {
        data_.lock()->to_actor_.SendDeferred(
            MakeWork(ManagerJobs::start_wallet));
    }
}

auto Shared::Wallet() const noexcept -> const node::Wallet& { return wallet_; }

Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::manager
