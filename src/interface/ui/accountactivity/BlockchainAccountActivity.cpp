// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accountactivity/BlockchainAccountActivity.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <PaymentEvent.pb.h>
#include <PaymentWorkflow.pb.h>
#include <PaymentWorkflowEnums.pb.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <iterator>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>

#include "internal/api/crypto/blockchain/Types.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/core/Factory.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/core/AccountType.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep
#include "opentxs/util/Writer.hpp"          // IWYU pragma: keep

namespace opentxs::factory
{
auto BlockchainAccountActivityModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Account& accountID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountActivity>
{
    using ReturnType = ui::implementation::BlockchainAccountActivity;

    if (AccountType::Blockchain != accountID.AccountType()) {
        LogAbort()("opentxs::factory::")(__func__)(
            ": wrong identifier type for ")(accountID.asHex())(": ")(
            print(accountID.Subtype()))
            .Abort();
    }

    const auto [chain, owner] =
        api.Crypto().Blockchain().LookupAccount(accountID);

    OT_ASSERT(owner == nymID);

    return std::make_unique<ReturnType>(api, chain, nymID, accountID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
using namespace std::literals;

BlockchainAccountActivity::BlockchainAccountActivity(
    const api::session::Client& api,
    const blockchain::Type chain,
    const identifier::Nym& nymID,
    const identifier::Account& accountID,
    const SimpleCallback& cb) noexcept
    : AccountActivity(api, nymID, accountID, AccountType::Blockchain, cb)
    , chain_(chain)
    , confirmed_(0)
    , balance_cb_(network::zeromq::ListenCallback::Factory(
          [this](auto&& in) { pipeline_.Push(std::move(in)); }))
    , balance_socket_(api_.Network().ZeroMQ().Internal().DealerSocket(
          balance_cb_,
          network::zeromq::socket::Direction::Connect,
          "BlockchainAccountActivity"))
    , progress_()
    , height_(0)
{
    const auto connected =
        balance_socket_->Start(api_.Endpoints().BlockchainBalance().data());

    OT_ASSERT(connected);

    init({
        UnallocatedCString{api.Endpoints().BlockchainReorg()},
        UnallocatedCString{api.Endpoints().BlockchainStateChange()},
        UnallocatedCString{api.Endpoints().BlockchainSyncProgress()},
        UnallocatedCString{api.Endpoints().BlockchainTransactions()},
        UnallocatedCString{api.Endpoints().BlockchainTransactions(nymID)},
        UnallocatedCString{api.Endpoints().ContactUpdate()},
    });
    balance_socket_->Send([&] {
        using Job = api::crypto::blockchain::BalanceOracleJobs;
        auto work = network::zeromq::tagged_message(Job::registration, true);
        work.AddFrame(chain_);
        work.AddFrame(nymID);

        return work;
    }());
}

auto BlockchainAccountActivity::DepositAddress(
    const blockchain::Type chain) const noexcept -> UnallocatedCString
{
    if ((blockchain::Type::UnknownBlockchain != chain) && (chain_ != chain)) {
        return {};
    }

    const auto& wallet =
        api_.Crypto().Blockchain().Account(primary_id_, chain_);
    const auto reason =
        api_.Factory().PasswordPrompt("Calculating next deposit address");

    if (auto s = blockchain::params::get(chain_).DefaultAddressStyle(); s) {

        return wallet.GetDepositAddress(*s, reason);
    } else {

        return {};
    }
}

auto BlockchainAccountActivity::display_balance(
    opentxs::Amount value) const noexcept -> UnallocatedCString
{
    return blockchain::internal::Format(chain_, value);
}

auto BlockchainAccountActivity::load_thread() noexcept -> void
{
    const auto transactions =
        [&]() -> UnallocatedVector<blockchain::block::TransactionHash> {
        try {
            const auto handle = api_.Network().Blockchain().GetChain(chain_);

            if (false == handle.IsValid()) {
                throw std::runtime_error{"invalid chain"};
            }

            const auto& chain = handle.get();
            height_ = chain.HeaderOracle().BestChain().height_;

            return chain.Internal().GetTransactions(primary_id_);
        } catch (...) {

            return {};
        }
    }();
    auto active = UnallocatedSet<AccountActivityRowID>{};

    for (const auto& txid : transactions) {
        if (const auto id = process_txid(txid); id.has_value()) {
            active.emplace(id.value());
        }
    }

    delete_inactive(active);
}

auto BlockchainAccountActivity::Notify(
    std::span<const PaymentCode> contacts) const noexcept -> bool
{
    try {
        const auto handle = api_.Network().Blockchain().GetChain(chain_);

        if (false == handle.IsValid()) {
            throw std::runtime_error{"invalid chain"};
        }

        const auto& network = handle.get();
        network.Sweep(primary_id_, ""sv, contacts);

        return true;
    } catch (...) {

        return false;
    }
}

auto BlockchainAccountActivity::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Payload();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body[0].as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::shutdown: {
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        case Work::contact: {
            process_contact(in);
        } break;
        case Work::balance: {
            process_balance(in);
        } break;
        case Work::new_block: {
            process_block(in);
        } break;
        case Work::txid: {
            process_txid(in);
        } break;
        case Work::reorg: {
            process_reorg(in);
        } break;
        case Work::statechange: {
            process_state(in);
        } break;
        case Work::sync: {
            process_sync(in);
        } break;
        case Work::init: {
            startup();
            finish_startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto BlockchainAccountActivity::print(Work type) noexcept -> const char*
{
    static const auto map = Map<Work, const char*>{
        {Work::shutdown, "shutdown"},
        {Work::contact, "contact"},
        {Work::balance, "balance"},
        {Work::new_block, "new_block"},
        {Work::txid, "txid"},
        {Work::reorg, "reorg"},
        {Work::statechange, "statechange"},
        {Work::sync, "sync"},
        {Work::init, "init"},
        {Work::statemachine, "statemachine"},
    };

    return map.at(type);
}

auto BlockchainAccountActivity::process_balance(const Message& in) noexcept
    -> void
{
    wait_for_startup();
    const auto body = in.Payload();

    OT_ASSERT(4 < body.size());

    const auto chain = body[1].as<blockchain::Type>();
    const auto confirmed = factory::Amount(body[2]);
    const auto unconfirmed = factory::Amount(body[3]);
    const auto nym = api_.Factory().NymIDFromHash(body[4].Bytes());

    OT_ASSERT(chain_ == chain);
    OT_ASSERT(primary_id_ == nym);

    const auto oldBalance = [&] {
        eLock lock(shared_lock_);

        const auto oldbalance = balance_;
        balance_ = unconfirmed;
        return oldbalance;
    }();
    const auto oldConfirmed = [&] {
        eLock lock(shared_lock_);

        const auto oldconfirmed = confirmed_;
        confirmed_ = confirmed;
        return oldconfirmed;
    }();

    if (oldBalance != unconfirmed) {
        notify_balance(unconfirmed);
    } else if (oldConfirmed != confirmed) {
        UpdateNotify();
    }

    load_thread();
}

auto BlockchainAccountActivity::process_block(const Message& in) noexcept
    -> void
{
    const auto body = in.Payload();

    OT_ASSERT(3 < body.size());

    const auto chain = body[1].as<blockchain::Type>();

    if (chain != chain_) { return; }

    process_height(body[3].as<blockchain::block::Height>());
}

auto BlockchainAccountActivity::process_contact(const Message& in) noexcept
    -> void
{
    wait_for_startup();
    const auto body = in.Payload();

    OT_ASSERT(1 < body.size());

    const auto contactID = api_.Factory()
                               .IdentifierFromProtobuf(body[1].Bytes())
                               .asBase58(api_.Crypto());
    const auto txids = [&] {
        auto out = UnallocatedSet<blockchain::block::TransactionHash>{};
        for_each_row([&](const auto& row) {
            for (const auto& id : row.Contacts()) {
                if (contactID == id) {
                    auto hash = blockchain::block::TransactionHash{};
                    blockchain::NumberToHash(
                        IsHex, row.UUID(), hash.WriteInto());
                    out.emplace(std::move(hash));

                    break;
                }
            }
        });

        return out;
    }();

    for (const auto& txid : txids) { process_txid(txid); }
}

auto BlockchainAccountActivity::process_height(
    const blockchain::block::Height height) noexcept -> void
{
    if (height == height_) { return; }

    height_ = height;
    load_thread();
}

auto BlockchainAccountActivity::process_reorg(const Message& in) noexcept
    -> void
{
    const auto body = in.Payload();

    OT_ASSERT(5 < body.size());

    const auto chain = body[1].as<blockchain::Type>();

    if (chain != chain_) { return; }

    process_height(body[5].as<blockchain::block::Height>());
}

auto BlockchainAccountActivity::process_state(const Message& in) noexcept
    -> void
{
    {
        const auto body = in.Payload();

        OT_ASSERT(2 < body.size());

        const auto chain = body[1].as<blockchain::Type>();

        if (chain_ != chain) { return; }

        const auto enabled = body[2].as<bool>();

        if (false == enabled) { return; }
    }

    try {
        const auto handle = api_.Network().Blockchain().GetChain(chain_);

        if (false == handle.IsValid()) {
            throw std::runtime_error{"invalid chain"};
        }

        const auto& chain = handle.get();
        process_height(chain.HeaderOracle().BestChain().height_);
    } catch (...) {
    }
}

auto BlockchainAccountActivity::process_sync(const Message& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(3 < body.size());

    const auto chain = body[1].as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto height = body[2].as<blockchain::block::Height>();
    const auto target = body[3].as<blockchain::block::Height>();

    OT_ASSERT(height <= std::numeric_limits<int>::max());
    OT_ASSERT(target <= std::numeric_limits<int>::max());

    const auto previous = progress_.get_progress();
    const auto current = static_cast<int>(height);
    const auto max = static_cast<int>(target);
    const auto percent = progress_.set(current, max);

    if (progress_.get_progress() != previous) {
        auto lock = Lock{callbacks_.lock_};
        const auto& cb = callbacks_.cb_.sync_;

        if (cb) { cb(current, max, percent); }

        UpdateNotify();
    }
}

auto BlockchainAccountActivity::process_txid(const Message& in) noexcept -> void
{
    wait_for_startup();
    const auto body = in.Payload();

    OT_ASSERT(3 < body.size());

    const auto& api = api_;
    const auto txid = blockchain::block::TransactionHash{body[1].Bytes()};
    const auto chain = body[2].as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto proto = proto::Factory<proto::BlockchainTransaction>(body[3]);
    process_txid(
        txid, api.Factory().InternalSession().BlockchainTransaction(proto, {}));
}

auto BlockchainAccountActivity::process_txid(
    const blockchain::block::TransactionHash& txid) noexcept
    -> std::optional<AccountActivityRowID>
{
    return process_txid(txid, api_.Crypto().Blockchain().LoadTransaction(txid));
}

auto BlockchainAccountActivity::process_txid(
    const blockchain::block::TransactionHash& txid,
    blockchain::block::Transaction tx) noexcept
    -> std::optional<AccountActivityRowID>
{
    const auto rowID = AccountActivityRowID{
        blockchain_thread_item_id(api_.Crypto(), api_.Factory(), chain_, txid),
        proto::PAYMENTEVENTTYPE_COMPLETE};

    if (false == tx.IsValid()) { return std::nullopt; }

    const auto& bTx = tx.asBitcoin();

    if (false == bTx.Chains({}).contains(chain_)) { return std::nullopt; }

    const auto sortKey{bTx.Timestamp()};
    const auto conf = [&]() -> int {
        const auto height = tx.Internal().asBitcoin().ConfirmationHeight();

        if ((0 > height) || (height > height_)) { return 0; }

        return static_cast<int>(height_ - height) + 1;
    }();
    auto description =
        api_.Crypto().Blockchain().ActivityDescription(primary_id_, chain_, tx);
    auto custom = CustomData{
        new proto::PaymentWorkflow(),
        new proto::PaymentEvent(),
        new blockchain::block::Transaction{std::move(tx)},
        new blockchain::Type{chain_},
        new UnallocatedCString{std::move(description)},
        new UnallocatedCString{txid.Bytes()},
        new int{conf},
    };
    add_item(rowID, sortKey, custom);

    return rowID;
}

auto BlockchainAccountActivity::Send(
    const UnallocatedCString& address,
    const Amount& amount,
    const std::string_view memo,
    std::span<const PaymentCode> notify) const noexcept -> bool
{
    try {
        const auto handle = api_.Network().Blockchain().GetChain(chain_);

        if (false == handle.IsValid()) {
            throw std::runtime_error{"invalid chain"};
        }

        const auto& network = handle.get();
        const auto recipient = api_.Factory().PaymentCodeFromBase58(address);

        if (0 < recipient.Version()) {
            network.SendToPaymentCode(
                primary_id_, recipient, amount, memo, notify);
        } else {
            const auto text = [&] {
                constexpr auto as_base58 = [](const auto& p) {
                    return p.asBase58();
                };
                auto out = Vector<std::string_view>{};
                out.reserve(notify.size());
                out.clear();
                std::transform(
                    notify.begin(),
                    notify.end(),
                    std::back_inserter(out),
                    as_base58);

                return out;
            }();
            network.SendToAddress(primary_id_, address, amount, memo, text);
        }

        return true;
    } catch (...) {

        return false;
    }
}

auto BlockchainAccountActivity::Send(
    const UnallocatedCString& address,
    const UnallocatedCString& amount,
    const std::string_view memo,
    Scale scale,
    std::span<const PaymentCode> notify) const noexcept -> bool
{
    const auto& definition = display::GetDefinition(blockchain_to_unit(chain_));

    if (const auto value = definition.Import(amount, scale); value) {

        return Send(address, *value, memo, notify);
    } else {

        return false;
    }
}

auto BlockchainAccountActivity::startup() noexcept -> void { load_thread(); }

auto BlockchainAccountActivity::ValidateAddress(
    const UnallocatedCString& in) const noexcept -> bool
{
    {
        const auto code = api_.Factory().PaymentCodeFromBase58(in);

        if (0 < code.Version()) { return true; }
    }

    using Style = blockchain::crypto::AddressStyle;

    const auto [data, style, chains, supported] =
        api_.Crypto().Blockchain().DecodeAddress(in);

    if (Style::Unknown == style) { return false; }

    if (0 == chains.count(chain_)) { return false; }

    return supported;
}

auto BlockchainAccountActivity::ValidateAmount(
    const UnallocatedCString& text) const noexcept -> UnallocatedCString
{
    const auto& definition = display::GetDefinition(blockchain_to_unit(chain_));

    if (const auto value = definition.Import(text); value) {

        return definition.Format(*value);
    } else {

        return {};
    }
}

BlockchainAccountActivity::~BlockchainAccountActivity()
{
    wait_for_startup();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
