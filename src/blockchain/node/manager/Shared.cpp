// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/manager/Shared.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <algorithm>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "blockchain/node/Mempool.hpp"
#include "blockchain/node/manager/SendPromises.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Factory.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/network/blockchain/Address.hpp"
#include "internal/network/blockchain/OTDHT.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Nym.hpp"
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
                OT_FAIL;
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
    OT_ASSERT(database_);
    OT_ASSERT(mempool_);
    OT_ASSERT(filter_);
}

auto Shared::AddBlock(const block::Block& block) const noexcept -> bool
{
    if (false == block.IsValid()) {
        LogError()(OT_PRETTY_CLASS())("invalid ")(print(chain_))(" block")
            .Flush();

        return false;
    }

    const auto& id = block.ID();

    if (false == block_.SubmitBlock(block, {})) {  // TODO monotonic allocator
        LogError()(OT_PRETTY_CLASS())("failed to save ")(print(chain_))(
            " block ")
            .asHex(id)
            .Flush();

        return false;
    }

    // TODO monotonic allocator
    if (false == filter_->Internal().ProcessBlock(block, {})) {
        LogError()(OT_PRETTY_CLASS())("failed to index ")(print(chain_))(
            " block")
            .Flush();

        return false;
    }

    if (false == header_.Internal().AddHeader(block.Header())) {
        LogError()(OT_PRETTY_CLASS())("failed to process ")(print(chain_))(
            " header")
            .Flush();

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

        return data_.lock()->to_peer_manager_.SendDeferred(
            std::move(work), __FILE__, __LINE__);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

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
        data.to_dht_.SendDeferred(
            [&] {
                auto out = network::zeromq::Message{};
                const auto command =
                    factory::BlockchainSyncPushTransaction(chain_, tx);
                command.Serialize(out);

                return out;
            }(),
            __FILE__,
            __LINE__);
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

    return data.to_peer_manager_.SendDeferred(
        std::move(message), __FILE__, __LINE__);
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

auto Shared::Finish(int index) noexcept -> std::promise<SendOutcome>
{
    return data_.lock()->send_promises_.finish(index);
}

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
    OT_ASSERT(self);

    header_.Internal().Init();
    auto api = api_.InternalClient().SharedClient();
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

        return data_.lock()->to_peer_manager_.SendDeferred(
            std::move(work), __FILE__, __LINE__);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

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

auto Shared::SendToAddress(
    const opentxs::identifier::Nym& sender,
    std::string_view address,
    const Amount amount,
    std::string_view memo,
    std::span<const std::string_view> notify) const noexcept
    -> Manager::PendingOutgoing
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto [index, future] = data.send_promises_.get();
    // TODO c++20
    data.to_actor_.SendDeferred(
        [&](const auto& i) {
            auto work = MakeWork(ManagerJobs::send_to_address);
            work.AddFrame(sender);
            work.AddFrame(address.data(), address.size());
            amount.Serialize(work.AppendBytes());
            work.AddFrame(memo.data(), memo.size());
            work.AddFrame(i);
            serialize_notifications(notify, work);

            return work;
        }(index),
        __FILE__,
        __LINE__);

    return std::move(future);
}

auto Shared::SendToPaymentCode(
    const opentxs::identifier::Nym& nymID,
    std::string_view recipient,
    const Amount amount,
    std::string_view memo,
    std::span<const std::string_view> notify) const noexcept
    -> Manager::PendingOutgoing
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto [index, future] = data.send_promises_.get();
    // TODO c++20
    data.to_actor_.SendDeferred(
        [&](const auto& i) {
            auto work = MakeWork(ManagerJobs::send_to_paymentcode);
            work.AddFrame(nymID);
            work.AddFrame(recipient.data(), recipient.size());
            amount.Serialize(work.AppendBytes());
            work.AddFrame(memo.data(), memo.size());
            work.AddFrame(i);
            serialize_notifications(notify, work);

            return work;
        }(index),
        __FILE__,
        __LINE__);

    return std::move(future);
}

auto Shared::SendToPaymentCode(
    const opentxs::identifier::Nym& nymID,
    const PaymentCode& recipient,
    const Amount amount,
    std::string_view memo,
    std::span<const PaymentCode> notify) const noexcept
    -> Manager::PendingOutgoing
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto [index, future] = data.send_promises_.get();
    // TODO c++20
    data.to_actor_.SendDeferred(
        [&](const auto& i) {
            auto work = MakeWork(ManagerJobs::send_to_paymentcode);
            work.AddFrame(nymID);
            work.AddFrame(recipient.asBase58());
            amount.Serialize(work.AppendBytes());
            work.AddFrame(memo.data(), memo.size());
            work.AddFrame(i);
            serialize_notifications(notify, work);

            return work;
        }(index),
        __FILE__,
        __LINE__);

    return std::move(future);
}

auto Shared::serialize_notifications(
    std::span<const std::string_view> in,
    network::zeromq::Message& out) noexcept -> void
{
    auto append_to_message = [&out](const auto& i) {
        out.AddFrame(i.data(), i.size());
    };
    std::for_each(in.begin(), in.end(), append_to_message);
}

auto Shared::serialize_notifications(
    std::span<const PaymentCode> in,
    network::zeromq::Message& out) noexcept -> void
{
    auto append_to_message = [&out](const auto& i) {
        out.AddFrame(i.asBase58());
    };
    std::for_each(in.begin(), in.end(), append_to_message);
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
            MakeWork(ManagerJobs::start_wallet), __FILE__, __LINE__);
    }
}

auto Shared::Sweep(
    const identifier::Nym& account,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept
    -> Manager::PendingOutgoing
{
    static const auto blankSubaccount = identifier::Account{};

    return sweep(account, blankSubaccount, std::nullopt, toAddress, notify);
}

auto Shared::Sweep(
    const identifier::Nym& account,
    const identifier::Account& subaccount,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept
    -> Manager::PendingOutgoing
{
    return sweep(account, subaccount, std::nullopt, toAddress, notify);
}

auto Shared::Sweep(
    const crypto::Key& key,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept
    -> Manager::PendingOutgoing
{
    const auto& [subaccount, subchain, index] = key;
    const auto [_, nym] = api_.Crypto().Blockchain().LookupAccount(subaccount);

    return sweep(nym, subaccount, key, toAddress, notify);
}

auto Shared::sweep(
    const identifier::Nym& account,
    const identifier::Account& subaccount,
    std::optional<crypto::Key> key,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept
    -> Manager::PendingOutgoing
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto [index, future] = data.send_promises_.get();
    // TODO c++20
    data.to_actor_.SendDeferred(
        [&](const auto& i) {
            auto work = MakeWork(ManagerJobs::sweep);
            account.Serialize(work);

            if (subaccount.empty()) {
                work.AddFrame();
            } else {
                subaccount.Serialize(work);
            }

            if (key.has_value()) {
                work.AddFrame(serialize(*key));
            } else {
                work.AddFrame();
            }

            work.AddFrame(toAddress.data(), toAddress.size());
            work.AddFrame(i);
            serialize_notifications(notify, work);

            return work;
        }(index),
        __FILE__,
        __LINE__);

    return std::move(future);
}

auto Shared::Wallet() const noexcept -> const node::Wallet& { return wallet_; }

Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::manager
