// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/rpc/Helpers.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <chrono>
#include <compare>
#include <future>
#include <iterator>
#include <mutex>
#include <string_view>
#include <utility>

#include "internal/api/session/UI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
using namespace std::literals::chrono_literals;
using namespace opentxs::literals;

RPC_fixture::SeedMap RPC_fixture::seed_map_{};
RPC_fixture::LocalNymMap RPC_fixture::local_nym_map_{};
RPC_fixture::IssuedUnits RPC_fixture::created_units_{};
RPC_fixture::AccountMap RPC_fixture::registered_accounts_{};
RPC_fixture::UserIndex RPC_fixture::users_{};

struct RPCPushCounter::Imp {
    auto at(std::size_t index) const -> const zmq::Message&
    {
        auto lock = ot::Lock{lock_};

        return *std::next(received_.begin(), index);
    }
    auto size() const noexcept -> std::size_t
    {
        auto lock = ot::Lock{lock_};

        return received_.size();
    }
    auto wait(std::size_t index) const noexcept -> bool
    {
        const auto condition = [&] { return size() > index; };

        if (condition()) { return true; }

        static constexpr auto limit = std::chrono::minutes{1};
        static constexpr auto wait = 100ms;
        const auto start = ot::Clock::now();

        while ((!condition()) && ((ot::Clock::now() - start) < limit)) {
            ot::Sleep(wait);
        }

        return condition();
    }

    Imp(const ot::api::Context& ot) noexcept
        : ot_(ot)
        , cb_(zmq::ListenCallback::Factory(
              [&](auto&& in) { cb(std::move(in)); }))
        , socket_(ot_.ZMQ().Internal().SubscribeSocket(cb_))
        , lock_()
        , received_()
    {
        const auto endpoint =
            ot::network::zeromq::MakeDeterministicInproc("rpc/push", -1, 1);
        socket_->Start(endpoint);
    }
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() { socket_->Close(); }

private:
    const ot::api::Context& ot_;
    const ot::OTZMQListenCallback cb_;
    const ot::OTZMQSubscribeSocket socket_;
    mutable std::mutex lock_;
    ot::UnallocatedList<ot::network::zeromq::Message> received_;

    auto cb(zmq::Message&& in) noexcept -> void
    {
        auto lock = ot::Lock{lock_};
        received_.emplace_back(std::move(in));
    }
};

RPC_fixture::RPC_fixture() noexcept
    : push_([&]() -> auto& {
        static auto out = RPCPushCounter{ot_};

        return out;
    }())
{
}

RPCPushCounter::RPCPushCounter(const ot::api::Context& ot) noexcept
    : imp_(std::make_unique<Imp>(ot))
{
}

auto RPCPushCounter::at(std::size_t index) const -> const zmq::Message&
{
    return imp_->at(index);
}

auto RPCPushCounter::shutdown() noexcept -> void { imp_.reset(); }

auto RPCPushCounter::size() const noexcept -> std::size_t
{
    return imp_->size();
}

auto RPCPushCounter::wait(std::size_t index) const noexcept -> bool
{
    return imp_->wait(index);
}

auto RPC_fixture::Cleanup() noexcept -> void
{
    push_.shutdown();
    registered_accounts_.clear();
    created_units_.clear();
    local_nym_map_.clear();
    seed_map_.clear();
}

auto RPC_fixture::CreateNym(
    const ot::api::session::Client& api,
    const ot::UnallocatedCString& name,
    const ot::UnallocatedCString& seed,
    int index) const noexcept -> const User&
{
    static auto counter = int{-1};
    const auto reason = api.Factory().PasswordPrompt(__func__);
    auto [it, added] = users_.try_emplace(
        ++counter,
        api.Crypto().Seed().Words(seed, reason),
        name,
        api.Crypto().Seed().Passphrase(seed, reason));

    OT_ASSERT(added);

    auto& user = it->second;
    user.init(api, ot::identity::Type::individual, index);
    auto& nym = user.nym_;

    OT_ASSERT(nym);

    const auto id = nym->ID().asBase58(ot_.Crypto());
    auto& nyms = local_nym_map_.at(api.Instance());
    nyms.emplace(id);

    return user;
}

auto RPC_fixture::DepositCheques(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::UnallocatedCString& nym) const noexcept -> std::size_t
{
    return DepositCheques(api, server, api.Factory().NymIDFromBase58(nym));
}

auto RPC_fixture::DepositCheques(
    const ot::api::session::Notary& server,
    const User& nym) const noexcept -> std::size_t
{
    return DepositCheques(*nym.api_, server, nym.nym_id_);
}

auto RPC_fixture::DepositCheques(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID) const noexcept -> std::size_t
{
    const auto& serverID = server.ID();
    const auto output = api.OTX().DepositCheques(nymID);

    if (0 == output) { return output; }

    RefreshAccount(api, nymID, serverID);

    return output;
}

auto RPC_fixture::ImportBip39(
    const ot::api::Session& api,
    const ot::UnallocatedCString& words) const noexcept
    -> ot::UnallocatedCString
{
    using SeedLang = ot::crypto::Language;
    using SeedStyle = ot::crypto::SeedStyle;
    auto& seeds = seed_map_[api.Instance()];
    const auto reason = api.Factory().PasswordPrompt(__func__);
    const auto [it, added] = seeds.emplace(api.Crypto().Seed().ImportSeed(
        ot_.Factory().SecretFromText(words),
        ot_.Factory().SecretFromText(""),
        SeedStyle::BIP39,
        SeedLang::en,
        reason));

    return *it;
}

auto RPC_fixture::ImportServerContract(
    const ot::api::session::Notary& from,
    const ot::api::session::Client& to) const noexcept -> bool
{
    const auto& id = from.ID();
    const auto server = from.Wallet().Internal().Server(id);

    if (0u == server->Version()) { return false; }

    auto bytes = ot::Space{};
    if (false == server->Serialize(ot::writer(bytes), true)) { return false; }
    const auto client = to.Wallet().Internal().Server(ot::reader(bytes));

    if (0u == client->Version()) { return false; }

    return id == client->ID();
}

auto RPC_fixture::init_maps(int instance) const noexcept -> void
{
    local_nym_map_[instance];
    seed_map_[instance];
}

auto RPC_fixture::InitAccountActivityCounter(
    const ot::api::session::Client& api,
    const ot::UnallocatedCString& nym,
    const ot::UnallocatedCString& account,
    Counter& counter) const noexcept -> void
{
    InitAccountActivityCounter(
        api, api.Factory().NymIDFromBase58(nym), account, counter);
}

auto RPC_fixture::InitAccountActivityCounter(
    const User& nym,
    const ot::UnallocatedCString& account,
    Counter& counter) const noexcept -> void
{
    InitAccountActivityCounter(*nym.api_, nym.nym_id_, account, counter);
}

auto RPC_fixture::InitAccountActivityCounter(
    const ot::api::session::Client& api,
    const ot::identifier::Nym& nym,
    const ot::UnallocatedCString& account,
    Counter& counter) const noexcept -> void
{
    api.UI().Internal().AccountActivity(
        nym,
        api.Factory().IdentifierFromBase58(account),
        make_cb(
            counter,
            ot::UnallocatedCString{u8"account activity "_sv} + account));
}

auto RPC_fixture::InitAccountTreeCounter(const User& nym, Counter& counter)
    const noexcept -> void
{
    InitAccountTreeCounter(*nym.api_, nym.nym_id_, counter);
}

auto RPC_fixture::InitAccountTreeCounter(
    const ot::api::session::Client& api,
    const ot::identifier::Nym& nym,
    Counter& counter) const noexcept -> void
{
    api.UI().Internal().AccountTree(
        nym,
        make_cb(
            counter,
            ot::UnallocatedCString{u8"account tree for "_sv} +
                nym.asBase58(ot_.Crypto())));
}

auto RPC_fixture::IssueUnit(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::UnallocatedCString& issuer,
    const ot::UnallocatedCString& shortname,
    const ot::UnallocatedCString& terms,
    ot::UnitType unitOfAccount,
    const ot::display::Definition& displayDefinition) const noexcept
    -> ot::UnallocatedCString
{
    return IssueUnit(
        api,
        server,
        api.Factory().NymIDFromBase58(issuer),
        shortname,
        terms,
        unitOfAccount,
        displayDefinition);
}

auto RPC_fixture::IssueUnit(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::UnallocatedCString& issuer,
    const ot::UnallocatedCString& shortname,
    const ot::UnallocatedCString& terms,
    ot::UnitType unitOfAccount) const noexcept -> ot::UnallocatedCString
{
    return IssueUnit(
        api,
        server,
        api.Factory().NymIDFromBase58(issuer),
        shortname,
        terms,
        unitOfAccount,
        ot::display::GetDefinition(unitOfAccount));
}

auto RPC_fixture::IssueUnit(
    const ot::api::session::Notary& server,
    const User& issuer,
    const ot::UnallocatedCString& shortname,
    const ot::UnallocatedCString& terms,
    ot::UnitType unitOfAccount,
    const ot::display::Definition& displayDefinition) const noexcept
    -> ot::UnallocatedCString
{
    return IssueUnit(
        *issuer.api_,
        server,
        issuer.nym_id_,
        shortname,
        terms,
        unitOfAccount,
        displayDefinition);
}

auto RPC_fixture::IssueUnit(
    const ot::api::session::Notary& server,
    const User& issuer,
    const ot::UnallocatedCString& shortname,
    const ot::UnallocatedCString& terms,
    ot::UnitType unitOfAccount) const noexcept -> ot::UnallocatedCString
{
    return IssueUnit(
        *issuer.api_,
        server,
        issuer.nym_id_,
        shortname,
        terms,
        unitOfAccount,
        ot::display::GetDefinition(unitOfAccount));
}

auto RPC_fixture::IssueUnit(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID,
    const ot::UnallocatedCString& shortname,
    const ot::UnallocatedCString& terms,
    ot::UnitType unitOfAccount) const noexcept -> ot::UnallocatedCString
{
    return IssueUnit(
        api,
        server,
        nymID,
        shortname,
        terms,
        unitOfAccount,
        ot::display::GetDefinition(unitOfAccount));
}

auto RPC_fixture::IssueUnit(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID,
    const ot::UnallocatedCString& shortname,
    const ot::UnallocatedCString& terms,
    ot::UnitType unitOfAccount,
    const ot::display::Definition& displayDefinition) const noexcept
    -> ot::UnallocatedCString
{
    const auto& serverID = server.ID();
    const auto reason = api.Factory().PasswordPrompt(__func__);
    const auto contract = api.Wallet().Internal().CurrencyContract(
        nymID.asBase58(ot_.Crypto()),
        shortname,
        terms,
        unitOfAccount,
        1,
        displayDefinition,
        reason);

    if (0u == contract->Version()) { return {}; }

    const auto& output =
        created_units_.emplace_back(contract->ID().asBase58(ot_.Crypto()));
    const auto unitID = api.Factory().UnitIDFromBase58(output);
    auto [taskID, future] = api.OTX().IssueUnitDefinition(
        nymID, serverID, unitID, unitOfAccount, "issuer account");

    if (0 == taskID) { return {}; }

    const auto [status, message] = future.get();

    if (ot::otx::LastReplyStatus::MessageSuccess != status) { return {}; }

    const auto& accountID =
        registered_accounts_[nymID.asBase58(ot_.Crypto())].emplace_back(
            message->acct_id_->Get());

    if (accountID.empty()) { return {}; }

    RefreshAccount(api, nymID, serverID);

    return output;
}

auto RPC_fixture::RefreshAccount(
    const ot::api::session::Client& api,
    const ot::identifier::Nym& nym,
    const ot::identifier::Notary& server) const noexcept -> void
{
    api.OTX().Refresh();
    api.OTX().ContextIdle(nym, server).get();
}

auto RPC_fixture::RefreshAccount(
    const ot::api::session::Client& api,
    const ot::UnallocatedVector<ot::UnallocatedCString> nyms,
    const ot::identifier::Notary& server) const noexcept -> void
{
    api.OTX().Refresh();

    for (const auto& nym : nyms) {
        api.OTX().ContextIdle(api.Factory().NymIDFromBase58(nym), server).get();
    }
}

auto RPC_fixture::RefreshAccount(
    const ot::api::session::Client& api,
    const ot::UnallocatedVector<const User*> nyms,
    const ot::identifier::Notary& server) const noexcept -> void
{
    api.OTX().Refresh();

    for (const auto& nym : nyms) {
        if (nullptr != nym) {
            api.OTX().ContextIdle(nym->nym_id_, server).get();
        }
    }
}

auto RPC_fixture::RegisterAccount(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::UnallocatedCString& nym,
    const ot::UnallocatedCString& unit,
    const ot::UnallocatedCString& label) const noexcept
    -> ot::UnallocatedCString
{
    return RegisterAccount(
        api, server, api.Factory().NymIDFromBase58(nym), unit, label);
}

auto RPC_fixture::RegisterAccount(
    const ot::api::session::Notary& server,
    const User& nym,
    const ot::UnallocatedCString& unit,
    const ot::UnallocatedCString& label) const noexcept
    -> ot::UnallocatedCString
{
    return RegisterAccount(*nym.api_, server, nym.nym_id_, unit, label);
}

auto RPC_fixture::RegisterAccount(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID,
    const ot::UnallocatedCString& unit,
    const ot::UnallocatedCString& label) const noexcept
    -> ot::UnallocatedCString
{
    const auto& serverID = server.ID();
    const auto unitID = api.Factory().UnitIDFromBase58(unit);
    auto [taskID, future] =
        api.OTX().RegisterAccount(nymID, serverID, unitID, label);

    if (0 == taskID) { return {}; }

    const auto [status, message] = future.get();

    if (ot::otx::LastReplyStatus::MessageSuccess != status) { return {}; }

    RefreshAccount(api, nymID, serverID);
    const auto& accountID =
        registered_accounts_[nymID.asBase58(ot_.Crypto())].emplace_back(
            message->acct_id_->Get());

    return accountID;
}

auto RPC_fixture::RegisterNym(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::UnallocatedCString& nymID) const noexcept -> bool
{
    return RegisterNym(api, server, api.Factory().NymIDFromBase58(nymID));
}

auto RPC_fixture::RegisterNym(
    const ot::api::session::Notary& server,
    const User& nym) const noexcept -> bool
{
    return RegisterNym(*nym.api_, server, nym.nym_id_);
}

auto RPC_fixture::RegisterNym(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID) const noexcept -> bool
{
    const auto& serverID = server.ID();
    auto [taskID, future] = api.OTX().RegisterNymPublic(nymID, serverID, true);

    if (0 == taskID) { return false; }

    const auto [status, message] = future.get();

    if (ot::otx::LastReplyStatus::MessageSuccess != status) { return false; }

    RefreshAccount(api, nymID, serverID);

    return true;
}

auto RPC_fixture::SendCheque(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::UnallocatedCString& nym,
    const ot::UnallocatedCString& account,
    const ot::UnallocatedCString& contact,
    const ot::UnallocatedCString& memo,
    Amount amount) const noexcept -> bool
{
    return SendCheque(
        api,
        server,
        api.Factory().NymIDFromBase58(nym),
        account,
        contact,
        memo,
        amount);
}

auto RPC_fixture::SendCheque(
    const ot::api::session::Notary& server,
    const User& nym,
    const ot::UnallocatedCString& account,
    const ot::UnallocatedCString& contact,
    const ot::UnallocatedCString& memo,
    Amount amount) const noexcept -> bool
{
    return SendCheque(
        *nym.api_, server, nym.nym_id_, account, contact, memo, amount);
}

auto RPC_fixture::SendCheque(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID,
    const ot::UnallocatedCString& account,
    const ot::UnallocatedCString& contact,
    const ot::UnallocatedCString& memo,
    Amount amount) const noexcept -> bool
{
    const auto& serverID = server.ID();
    const auto accountID = api.Factory().IdentifierFromBase58(account);
    const auto contactID = api.Factory().IdentifierFromBase58(contact);
    auto [taskID, future] =
        api.OTX().SendCheque(nymID, accountID, contactID, amount, memo);

    if (0 == taskID) { return false; }

    const auto [status, message] = future.get();

    if (ot::otx::LastReplyStatus::MessageSuccess != status) { return false; }

    RefreshAccount(api, nymID, serverID);

    return true;
}

auto RPC_fixture::SendTransfer(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::UnallocatedCString& sender,
    const ot::UnallocatedCString& fromAccount,
    const ot::UnallocatedCString& toAccount,
    const ot::UnallocatedCString& memo,
    Amount amount) const noexcept -> bool
{
    return SendTransfer(
        api,
        server,
        api.Factory().NymIDFromBase58(sender),
        fromAccount,
        toAccount,
        memo,
        std::move(amount));
}

auto RPC_fixture::SendTransfer(
    const ot::api::session::Notary& server,
    const User& sender,
    const ot::UnallocatedCString& fromAccount,
    const ot::UnallocatedCString& toAccount,
    const ot::UnallocatedCString& memo,
    Amount amount) const noexcept -> bool
{
    return SendTransfer(
        *sender.api_,
        server,
        sender.nym_id_,
        fromAccount,
        toAccount,
        memo,
        std::move(amount));
}

auto RPC_fixture::SendTransfer(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID,
    const ot::UnallocatedCString& fromAccount,
    const ot::UnallocatedCString& toAccount,
    const ot::UnallocatedCString& memo,
    Amount amount) const noexcept -> bool
{
    const auto& serverID = server.ID();
    const auto from = api.Factory().IdentifierFromBase58(fromAccount);
    const auto to = api.Factory().IdentifierFromBase58(toAccount);
    auto [taskID, future] =
        api.OTX().SendTransfer(nymID, serverID, from, to, amount, memo);

    if (0 == taskID) { return false; }

    const auto [status, message] = future.get();

    if (ot::otx::LastReplyStatus::MessageSuccess != status) { return false; }

    RefreshAccount(api, nymID, serverID);

    return true;
}

auto RPC_fixture::SetIntroductionServer(
    const ot::api::session::Client& on,
    const ot::api::session::Notary& to) const noexcept -> bool
{
    const auto& id = to.ID();

    if (false == ImportServerContract(to, on)) { return false; }

    const auto clientID =
        on.OTX().SetIntroductionServer(on.Wallet().Internal().Server(id));

    return id == clientID;
}

auto RPC_fixture::StartClient(int index) const noexcept
    -> const ot::api::session::Client&
{
    const auto& out = ot_.StartClientSession(index);
    init_maps(out.Instance());

    return out;
}

auto RPC_fixture::StartNotarySession(int index) const noexcept
    -> const ot::api::session::Notary&
{
    const auto& out = ot_.StartNotarySession(index);
    const auto instance = out.Instance();
    init_maps(instance);
    auto& nyms = local_nym_map_.at(instance);
    const auto reason = out.Factory().PasswordPrompt(__func__);
    const auto lNyms = out.Wallet().LocalNyms();
    std::transform(
        lNyms.begin(),
        lNyms.end(),
        std::inserter(nyms, nyms.end()),
        [this](const auto& in) { return in.asBase58(ot_.Crypto()); });
    auto& seeds = seed_map_.at(instance);
    seeds.emplace(out.Crypto().Seed().DefaultSeed().first);

    return out;
}

RPCPushCounter::~RPCPushCounter() = default;
}  // namespace ottest
