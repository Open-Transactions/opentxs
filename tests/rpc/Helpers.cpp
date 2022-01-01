// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "rpc/Helpers.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <future>
#include <iterator>
#include <list>
#include <mutex>
#include <utility>

#include "integration/Helpers.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/rpc/AccountData.hpp"
#include "opentxs/rpc/AccountEvent.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/GetAccountActivity.hpp"
#include "opentxs/rpc/request/GetAccountBalance.hpp"
#include "opentxs/rpc/request/ListAccounts.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/GetAccountActivity.hpp"
#include "opentxs/rpc/response/GetAccountBalance.hpp"
#include "opentxs/rpc/response/ListAccounts.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "ui/Helpers.hpp"

namespace ottest
{
auto verify_account_balance(
    const int index,
    const std::string& account,
    const ot::Amount required) noexcept -> bool;
auto verify_response_codes(
    const ot::rpc::response::Base::Responses& codes,
    const std::size_t count,
    const ot::rpc::ResponseCode required =
        ot::rpc::ResponseCode::success) noexcept -> bool;

RPC_fixture::SeedMap RPC_fixture::seed_map_{};
RPC_fixture::LocalNymMap RPC_fixture::local_nym_map_{};
RPC_fixture::IssuedUnits RPC_fixture::created_units_{};
RPC_fixture::AccountMap RPC_fixture::registered_accounts_{};
RPC_fixture::UserIndex RPC_fixture::users_{};

auto check_account_activity_rpc(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool
{
    auto output{true};
    const auto& api = *user.api_;
    const auto index{api.Instance()};
    const auto command =
        ot::rpc::request::GetAccountActivity{index, {account.str()}};
    const auto base = ot::Context().RPC(command);
    const auto& response = base->asGetAccountActivity();
    const auto& codes = response.ResponseCodes();
    const auto& events = response.Activity();
    const auto vCount = expected.rows_.size();
    const auto goodCodes = verify_response_codes(
        codes,
        1,
        (0 == vCount) ? ot::rpc::ResponseCode::none
                      : ot::rpc::ResponseCode::success);
    output &= verify_account_balance(index, account.str(), expected.balance_);
    output &= goodCodes;
    output &= (events.size() == vCount);

    EXPECT_EQ(events.size(), vCount);

    auto v{expected.rows_.begin()};
    auto t{events.begin()};

    for (; (v != expected.rows_.end()) && (t != events.end()); ++v, ++t) {
        const auto& event = *t;
        const auto& required = *v;

        // TODO event.ContactID()
        // TODO event.State()
        // TODO event.Type()
        output &= (event.AccountID() == expected.id_);
        output &= (event.ConfirmedAmount() == required.amount_);
        output &= (event.Memo() == required.memo_);
        output &= (event.PendingAmount() == required.amount_);
        output &= (event.UUID() == required.uuid_);
        output &= (event.WorkflowID() == required.workflow_);

        EXPECT_EQ(event.AccountID(), expected.id_);
        EXPECT_EQ(event.ConfirmedAmount(), required.amount_);
        EXPECT_EQ(event.Memo(), required.memo_);
        EXPECT_EQ(event.PendingAmount(), required.amount_);
        EXPECT_EQ(event.UUID(), required.uuid_);
        EXPECT_EQ(event.WorkflowID(), required.workflow_);
    }

    return output;
}

auto check_account_list_rpc(
    const User& user,
    const AccountListData& expected) noexcept -> bool
{
    auto output{true};
    const auto& api = *user.api_;
    const auto index{api.Instance()};
    const auto command = ot::rpc::request::ListAccounts{index};
    const auto base = ot::Context().RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto& ids = response.AccountIDs();
    const auto goodCodes = verify_response_codes(codes, 1);
    const auto required = [&] {
        auto out = std::pmr::vector<std::string>{};
        out.reserve(expected.rows_.size());

        for (const auto& row : expected.rows_) {
            out.emplace_back(row.account_id_);
        }

        std::sort(out.begin(), out.end());

        return out;
    }();
    const auto got = [&] {
        auto out{ids};
        std::sort(out.begin(), out.end());

        return out;
    }();
    output &= goodCodes;
    output &= (got == required);

    EXPECT_TRUE(goodCodes);
    EXPECT_EQ(got, required);

    return output;
}

auto verify_account_balance(
    const int index,
    const std::string& account,
    const ot::Amount required) noexcept -> bool
{
    auto output{true};
    const auto command = ot::rpc::request::GetAccountBalance{index, {account}};
    const auto base = ot::Context().RPC(command);
    const auto& response = base->asGetAccountBalance();
    const auto& codes = response.ResponseCodes();
    const auto& data = response.Balances();
    const auto goodCodes = verify_response_codes(codes, 1);
    output &= goodCodes;
    output &= (data.size() == 1);

    EXPECT_TRUE(goodCodes);
    EXPECT_EQ(data.size(), 1);

    if (0 < data.size()) {
        const auto& item = data.front();

        // TODO item.ConfirmedBalance()
        // TODO item.ID()
        // TODO item.Issuer()
        // TODO item.Name()
        // TODO item.Owner()
        // TODO item.Type()
        // TODO item.Unit()

        output &= (item.PendingBalance() == required);

        EXPECT_EQ(item.PendingBalance(), required);
    }

    return output;
}

auto verify_response_codes(
    const ot::rpc::response::Base::Responses& codes,
    const std::size_t count,
    const ot::rpc::ResponseCode required) noexcept -> bool
{
    auto output{true};
    output &= (codes.size() == count);

    EXPECT_EQ(codes.size(), count);

    for (auto i = std::size_t{}; i < codes.size(); ++i) {
        const auto& code = codes.at(i);
        output &= (code.first == i);
        output &= (code.second == required);

        EXPECT_EQ(code.first, i);
        EXPECT_EQ(code.second, required);
    }

    return output;
}

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
        static constexpr auto wait = std::chrono::milliseconds{100};
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
        , socket_(ot_.ZMQ().SubscribeSocket(cb_))
        , lock_()
        , received_()
    {
        const auto endpoint =
            ot::network::zeromq::MakeDeterministicInproc("rpc/push", -1, 1);
        socket_->Start(endpoint);
    }

    ~Imp() { socket_->Close(); }

private:
    const ot::api::Context& ot_;
    const ot::OTZMQListenCallback cb_;
    const ot::OTZMQSubscribeSocket socket_;
    mutable std::mutex lock_;
    std::pmr::list<ot::network::zeromq::Message> received_;

    auto cb(zmq::Message&& in) noexcept -> void
    {
        auto lock = ot::Lock{lock_};
        received_.emplace_back(std::move(in));
    }

    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

RPC_fixture::RPC_fixture() noexcept
    : ot_(ot::Context())
    , push_([&]() -> auto& {
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
    const std::string& name,
    const std::string& seed,
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
    user.init(api, ot::contact::ClaimType::Individual, index);
    auto& nym = user.nym_;

    OT_ASSERT(nym);

    const auto id = nym->ID().str();
    auto& nyms = local_nym_map_.at(api.Instance());
    nyms.emplace(id);

    return user;
}

auto RPC_fixture::DepositCheques(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const std::string& nym) const noexcept -> std::size_t
{
    return DepositCheques(api, server, api.Factory().NymID(nym));
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
    const std::string& words) const noexcept -> std::string
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
    const auto server = from.Wallet().Server(id);

    if (0u == server->Version()) { return false; }

    auto bytes = ot::Space{};
    if (false == server->Serialize(ot::writer(bytes), true)) { return false; }
    const auto client = to.Wallet().Server(ot::reader(bytes));

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
    const std::string& nym,
    const std::string& account,
    Counter& counter) const noexcept -> void
{
    InitAccountActivityCounter(api, api.Factory().NymID(nym), account, counter);
}

auto RPC_fixture::InitAccountActivityCounter(
    const User& nym,
    const std::string& account,
    Counter& counter) const noexcept -> void
{
    InitAccountActivityCounter(*nym.api_, nym.nym_id_, account, counter);
}

auto RPC_fixture::InitAccountActivityCounter(
    const ot::api::session::Client& api,
    const ot::identifier::Nym& nym,
    const std::string& account,
    Counter& counter) const noexcept -> void
{
    api.UI().AccountActivity(
        nym,
        api.Factory().IdentifierFromBase58(account),
        make_cb(counter, std::string{u8"account activity "} + account));
}

auto RPC_fixture::IssueUnit(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const std::string& issuer,
    const std::string& shortname,
    const std::string& terms,
    ot::core::UnitType unitOfAccount,
    const ot::display::Definition& displayDefinition) const noexcept
    -> std::string
{
    return IssueUnit(
        api,
        server,
        api.Factory().NymID(issuer),
        shortname,
        terms,
        unitOfAccount,
        displayDefinition);
}

auto RPC_fixture::IssueUnit(
    const ot::api::session::Notary& server,
    const User& issuer,
    const std::string& shortname,
    const std::string& terms,
    ot::core::UnitType unitOfAccount,
    const ot::display::Definition& displayDefinition) const noexcept
    -> std::string
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
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID,
    const std::string& shortname,
    const std::string& terms,
    ot::core::UnitType unitOfAccount,
    const ot::display::Definition& displayDefinition) const noexcept
    -> std::string
{
    const auto& serverID = server.ID();
    const auto reason = api.Factory().PasswordPrompt(__func__);
    const auto contract = api.Wallet().CurrencyContract(
        nymID.str(),
        shortname,
        terms,
        unitOfAccount,
        1,
        displayDefinition,
        reason);

    if (0u == contract->Version()) { return {}; }

    const auto& output = created_units_.emplace_back(contract->ID()->str());
    const auto unitID = api.Factory().UnitID(output);
    auto [taskID, future] = api.OTX().IssueUnitDefinition(
        nymID, serverID, unitID, unitOfAccount, "issuer");

    if (0 == taskID) { return {}; }

    const auto [status, message] = future.get();

    if (ot::otx::LastReplyStatus::MessageSuccess != status) { return {}; }

    const auto& accountID = registered_accounts_[nymID.str()].emplace_back(
        message->m_strAcctID->Get());

    if (accountID.empty()) { return {}; }

    RefreshAccount(api, nymID, serverID);

    return output;
}

auto RPC_fixture::RefreshAccount(
    const ot::api::session::Client& api,
    const ot::identifier::Nym& nym,
    const ot::identifier::Server& server) const noexcept -> void
{
    api.OTX().Refresh();
    api.OTX().ContextIdle(nym, server).get();
}

auto RPC_fixture::RefreshAccount(
    const ot::api::session::Client& api,
    const std::pmr::vector<std::string> nyms,
    const ot::identifier::Server& server) const noexcept -> void
{
    api.OTX().Refresh();

    for (const auto& nym : nyms) {
        api.OTX().ContextIdle(api.Factory().NymID(nym), server).get();
    }
}

auto RPC_fixture::RefreshAccount(
    const ot::api::session::Client& api,
    const std::pmr::vector<const User*> nyms,
    const ot::identifier::Server& server) const noexcept -> void
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
    const std::string& nym,
    const std::string& unit,
    const std::string& label) const noexcept -> std::string
{
    return RegisterAccount(api, server, api.Factory().NymID(nym), unit, label);
}

auto RPC_fixture::RegisterAccount(
    const ot::api::session::Notary& server,
    const User& nym,
    const std::string& unit,
    const std::string& label) const noexcept -> std::string
{
    return RegisterAccount(*nym.api_, server, nym.nym_id_, unit, label);
}

auto RPC_fixture::RegisterAccount(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID,
    const std::string& unit,
    const std::string& label) const noexcept -> std::string
{
    const auto& serverID = server.ID();
    const auto unitID = api.Factory().UnitID(unit);
    auto [taskID, future] =
        api.OTX().RegisterAccount(nymID, serverID, unitID, label);

    if (0 == taskID) { return {}; }

    const auto [status, message] = future.get();

    if (ot::otx::LastReplyStatus::MessageSuccess != status) { return {}; }

    RefreshAccount(api, nymID, serverID);
    const auto& accountID = registered_accounts_[nymID.str()].emplace_back(
        message->m_strAcctID->Get());

    return accountID;
}

auto RPC_fixture::RegisterNym(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const std::string& nymID) const noexcept -> bool
{
    return RegisterNym(api, server, api.Factory().NymID(nymID));
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
    const std::string& nym,
    const std::string& account,
    const std::string& contact,
    const std::string& memo,
    Amount amount) const noexcept -> bool
{
    return SendCheque(
        api, server, api.Factory().NymID(nym), account, contact, memo, amount);
}

auto RPC_fixture::SendCheque(
    const ot::api::session::Notary& server,
    const User& nym,
    const std::string& account,
    const std::string& contact,
    const std::string& memo,
    Amount amount) const noexcept -> bool
{
    return SendCheque(
        *nym.api_, server, nym.nym_id_, account, contact, memo, amount);
}

auto RPC_fixture::SendCheque(
    const ot::api::session::Client& api,
    const ot::api::session::Notary& server,
    const ot::identifier::Nym& nymID,
    const std::string& account,
    const std::string& contact,
    const std::string& memo,
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
    const std::string& sender,
    const std::string& fromAccount,
    const std::string& toAccount,
    const std::string& memo,
    Amount amount) const noexcept -> bool
{
    return SendTransfer(
        api,
        server,
        api.Factory().NymID(sender),
        fromAccount,
        toAccount,
        memo,
        std::move(amount));
}

auto RPC_fixture::SendTransfer(
    const ot::api::session::Notary& server,
    const User& sender,
    const std::string& fromAccount,
    const std::string& toAccount,
    const std::string& memo,
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
    const std::string& fromAccount,
    const std::string& toAccount,
    const std::string& memo,
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
        on.OTX().SetIntroductionServer(on.Wallet().Server(id));

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
        [](const auto& in) { return in->str(); });
    auto& seeds = seed_map_.at(instance);
    seeds.emplace(out.Crypto().Seed().DefaultSeed());

    return out;
}

RPCPushCounter::~RPCPushCounter() = default;
}  // namespace ottest
