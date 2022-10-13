// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "interface/ui/accountlist/AccountList.hpp"  // IWYU pragma: associated

#include <atomic>
#include <future>
#include <memory>
#include <string_view>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "internal/api/crypto/blockchain/Types.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/core/AccountType.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto AccountListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountList>
{
    using ReturnType = ui::implementation::AccountList;

    return std::make_unique<ReturnType>(api, nymID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountList::AccountList(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : AccountListList(api, nymID, cb, false)
    , Worker(api, {}, "ui::AccountList")
    , chains_()
{
    // TODO monitor for notary nym changes since this may affect custodial
    // account names
    init_executor({
        UnallocatedCString{api.Endpoints().AccountUpdate()},
        UnallocatedCString{api.Endpoints().BlockchainAccountCreated()},
    });
    pipeline_.ConnectDealer(api.Endpoints().BlockchainBalance(), [](auto) {
        return MakeWork(Work::init);
    });
}

auto AccountList::construct_row(
    const AccountListRowID& id,
    const AccountListSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::AccountListItem(*this, api_, id, index, custom);
}

auto AccountList::load_blockchain() noexcept -> void
{
    const auto& blockchain = api_.Crypto().Blockchain();

    for (const auto& account : blockchain.AccountList(primary_id_)) {
        load_blockchain_account(
            std::move(const_cast<identifier::Generic&>(account)));
    }
}

auto AccountList::load_blockchain_account(identifier::Generic&& id) noexcept
    -> void
{
    const auto [chain, owner] = api_.Crypto().Blockchain().LookupAccount(id);

    OT_ASSERT(blockchain::Type::Unknown != chain);
    OT_ASSERT(owner == primary_id_);

    load_blockchain_account(std::move(id), chain);
}

auto AccountList::load_blockchain_account(blockchain::Type chain) noexcept
    -> void
{
    load_blockchain_account(
        identifier::Generic{
            api_.Crypto().Blockchain().Account(primary_id_, chain).AccountID()},
        chain,
        {});
}

auto AccountList::load_blockchain_account(
    identifier::Generic&& id,
    blockchain::Type chain) noexcept -> void
{
    load_blockchain_account(std::move(id), chain, {});
}

auto AccountList::load_blockchain_account(
    identifier::Generic&& id,
    blockchain::Type chain,
    Amount&& balance) noexcept -> void
{
    LogInsane()(OT_PRETTY_CLASS())("processing blockchain account ")(id)
        .Flush();
    const auto type = BlockchainToUnit(chain);
    const auto& api = api_;
    const auto index = AccountListSortKey{type, account_name_blockchain(chain)};
    auto custom = [&] {
        auto out = CustomData{};
        out.reserve(4);
        out.emplace_back(
            std::make_unique<AccountType>(AccountType::Blockchain).release());
        out.emplace_back(std::make_unique<identifier::UnitDefinition>(
                             blockchain::UnitID(api, chain))
                             .release());
        out.emplace_back(std::make_unique<identifier::Notary>(
                             blockchain::NotaryID(api, chain))
                             .release());
        out.emplace_back(
            std::make_unique<Amount>(std::move(balance)).release());

        return out;
    }();
    add_item(id, index, custom);
    if (chains_.cend() == chains_.find(chain)) {
        subscribe(chain);
        chains_.emplace(chain);
    }
}

auto AccountList::load_custodial() noexcept -> void
{
    const auto& storage = api_.Storage();

    for (const auto& account : storage.AccountsByOwner(primary_id_)) {
        load_custodial_account(
            std::move(const_cast<identifier::Generic&>(account)));
    }
}

auto AccountList::load_custodial_account(identifier::Generic&& id) noexcept
    -> void
{
    const auto& wallet = api_.Wallet();
    auto account = wallet.Internal().Account(id);
    const auto& contractID = account.get().GetInstrumentDefinitionID();
    const auto contract = wallet.Internal().UnitDefinition(contractID);
    load_custodial_account(
        std::move(id),
        identifier::UnitDefinition{contractID},
        contract->UnitOfAccount(),
        account.get().GetBalance(),
        account.get().Alias());
}

auto AccountList::load_custodial_account(
    identifier::Generic&& id,
    Amount&& balance) noexcept -> void
{
    const auto& wallet = api_.Wallet();
    auto account = wallet.Internal().Account(id);
    const auto& contractID = account.get().GetInstrumentDefinitionID();
    const auto contract = wallet.Internal().UnitDefinition(contractID);
    load_custodial_account(
        std::move(id),
        identifier::UnitDefinition{contractID},
        contract->UnitOfAccount(),
        std::move(balance),
        account.get().Alias());
}

auto AccountList::load_custodial_account(
    identifier::Generic&& id,
    identifier::UnitDefinition&& contract,
    UnitType type,
    Amount&& balance,
    UnallocatedCString&& name) noexcept -> void
{
    LogInsane()(OT_PRETTY_CLASS())("processing custodial account ")(id).Flush();
    const auto& api = api_;
    auto notaryID = api.Storage().AccountServer(id);
    const auto index = AccountListSortKey{
        type, account_name_custodial(api, notaryID, contract, std::move(name))};
    auto custom = [&] {
        auto out = CustomData{};
        out.reserve(4);
        out.emplace_back(
            std::make_unique<AccountType>(AccountType::Custodial).release());
        out.emplace_back(
            std::make_unique<identifier::UnitDefinition>(std::move(contract))
                .release());
        out.emplace_back(
            std::make_unique<identifier::Notary>(std::move(notaryID))
                .release());
        out.emplace_back(
            std::make_unique<Amount>(std::move(balance)).release());

        return out;
    }();
    add_item(id, index, custom);
}

auto AccountList::pipeline(Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    if ((false == startup_complete()) && (Work::init != work)) {
        pipeline_.Push(std::move(in));

        return;
    }

    switch (work) {
        case Work::shutdown: {
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        case Work::custodial: {
            process_custodial(std::move(in));
        } break;
        case Work::blockchain: {
            process_blockchain(std::move(in));
        } break;
        case Work::balance: {
            process_blockchain_balance(std::move(in));
        } break;
        case Work::init: {
            startup();
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

auto AccountList::print(Work type) noexcept -> const char*
{
    static const auto map = Map<Work, const char*>{
        {Work::shutdown, "shutdown"},
        {Work::custodial, "custodial"},
        {Work::blockchain, "blockchain"},
        {Work::balance, "balance"},
        {Work::init, "init"},
        {Work::statemachine, "statemachine"},
    };

    return map.at(type);
}

auto AccountList::process_blockchain(Message&& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(4 < body.size());

    const auto nymID = api_.Factory().NymIDFromHash(body.at(2).Bytes());

    if (nymID != primary_id_) {
        LogInsane()(OT_PRETTY_CLASS())("Update does not apply to this widget")
            .Flush();

        return;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    OT_ASSERT(blockchain::Type::Unknown != chain);

    load_blockchain_account(chain);
}

auto AccountList::process_blockchain_balance(Message&& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();
    const auto& accountID =
        api_.Crypto().Blockchain().Account(primary_id_, chain).AccountID();
    load_blockchain_account(
        identifier::Generic{accountID}, chain, factory::Amount(body.at(3)));
}

auto AccountList::process_custodial(Message&& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(2 < body.size());

    const auto& api = api_;
    auto id = api.Factory().IdentifierFromHash(body.at(1).Bytes());
    const auto owner = api.Storage().AccountOwner(id);

    if (owner != primary_id_) { return; }

    load_custodial_account(std::move(id), factory::Amount(body.at(2).Bytes()));
}

auto AccountList::startup() noexcept -> void
{
    load_blockchain();
    load_custodial();
    finish_startup();
    trigger();
}

auto AccountList::subscribe(const blockchain::Type chain) const noexcept -> void
{
    pipeline_.Send([&] {
        using Job = api::crypto::blockchain::BalanceOracleJobs;
        auto work = network::zeromq::tagged_message(Job::registration);
        work.AddFrame(chain);
        work.AddFrame(primary_id_);

        return work;
    }());
}

AccountList::~AccountList()
{
    wait_for_startup();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
